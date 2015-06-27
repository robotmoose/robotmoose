package iu.slam.websockets;

import iu.slam.actions.robot.RobotConnections;
import iu.slam.database.DatabaseHandler;
import iu.slam.models.Experiment;
import iu.slam.models.RobotInfo;
import iu.slam.models.SensorData;
import iu.slam.protocol.ControlMessage;
import iu.slam.utils.MyContextListener;
import iu.slam.utils.UtilLog;
import java.io.File;
import java.io.IOException;
import java.util.ArrayList;
import java.util.Collections;
import java.util.Date;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.atomic.AtomicBoolean;
import javax.websocket.CloseReason;
import javax.websocket.OnClose;
import javax.websocket.OnError;
import javax.websocket.OnMessage;
import javax.websocket.OnOpen;
import javax.websocket.Session;
import javax.websocket.server.ServerEndpoint;
import org.apache.commons.io.output.FileWriterWithEncoding;

/**
 *
 * @author hanaldo
 */
@ServerEndpoint(value = "/datalog")
public class EndpointDataLog {

    private static final Set<Session> clients = Collections.synchronizedSet(new HashSet<Session>(10));
    private static final Map<String, FileWriterWithEncoding> dialogFiles = new ConcurrentHashMap<>(10);
    private static final Map<String, Long> dialogs = new ConcurrentHashMap<>(10);
    private static String experimentId, experimentName;
    private static final AtomicBoolean pauseOnMessage = new AtomicBoolean(false);

    public static void broadcast(String message) throws IOException {
        //UtilLog.logInfo(EndpointDataLog.class, "Broadcast: " + message);
        for (Session client : clients) {
            client.getBasicRemote().sendText(message);
        }
    }

    public static int currentConnections() {
        return clients.size();
    }

    public static synchronized void setExperimentId(String experimentId) {
        EndpointDataLog.experimentId = experimentId;
    }

    public static synchronized String getExperimentId() {
        return experimentId;
    }

    public static String getExperimentName() {
        return experimentName;
    }

    public static boolean isRecordingStarted() {
        return getExperimentId() != null;
    }

    public static boolean startRecording(String name) {
        pauseOnMessage.set(true);
        experimentName = name;
        try {
            if (getExperimentId() == null) {
                endAllDialogs();
                setExperimentId(String.valueOf(System.currentTimeMillis()));
                UtilLog.logInfo(EndpointDataLog.class, "Recording started");
                UtilLog.logInfo(EndpointDataLog.class, "Current experimentId: " + experimentId);
                UtilLog.logInfo(EndpointDataLog.class, "Current experimentName: " + experimentName);

                return true;
            } else {
                return false;
            }
        } catch (Exception ex) {
            UtilLog.logWarn(EndpointDataLog.class, ex);
            return false;
        } finally {
            pauseOnMessage.set(false);
        }
    }

    public static synchronized void endAllDialogs() {
        pauseOnMessage.set(true);
        try {
            if (clients.isEmpty()) {
                checkExperimentEnding();
            }
            for (Session client : clients) {
                endClientDialog(client.getId(), true);
            }
        } finally {
            pauseOnMessage.set(false);
        }
    }

    private static void checkExperimentEnding() {
        if (experimentId != null) {
            //if no dialogs, we will then end the current experiment
            if (dialogFiles.isEmpty()) {
                setExperimentId(null);
                UtilLog.logInfo(EndpointDataLog.class, "Recording ended");
            }
        }
    }

    private static synchronized void makeNewClientDialog(String clientId, long currentTime) throws IOException {
        String filename = MyContextListener.getContextPath() + "files/" + currentTime + ".txt";
        File file = new File(filename);
        file.createNewFile();
        UtilLog.logInfo(EndpointDataLog.class, "new file: " + filename);
        FileWriterWithEncoding writer = new FileWriterWithEncoding(file, "utf-8");
        dialogFiles.put(clientId, writer);
        dialogs.put(clientId, currentTime);
        TimerCheckDataIdle.getCurrentTask().addDialog(clientId);

        if (experimentId != null) {
            org.hibernate.Session s = DatabaseHandler.getSession();
            try {
                if (!Experiment.isExperimentStarted(experimentId, s)) {
                    Experiment.addNewExperiment(experimentId, new Date(currentTime), experimentName, s);
                }
            } catch (Exception e) {
                UtilLog.logWarn(EndpointDataLog.class, e);
            } finally {
                DatabaseHandler.closeSession(s);
            }
        }

        reportTotalDialogs();
    }

    private synchronized static void endClientDialog(String clientId, boolean reportExpEnd) {
        if (dialogFiles.containsKey(clientId)) {
            try {
                //close file writer
                dialogFiles.get(clientId).close();
            } catch (IOException ex) {
                UtilLog.logWarn(EndpointDataLog.class, ex);
            }

            //save dialog record to database
            org.hibernate.Session s = DatabaseHandler.getSession();
            try {
                String filePath = "files/" + dialogs.get(clientId) + ".txt";
                Date endTime = new Date();
                int rid = RobotConnections.getRobotId(clientId);
                if (rid < 0) {
                    UtilLog.logError(EndpointDataLog.class, "We lost the robot id for " + clientId);
                }
                SensorData data = new SensorData(filePath, rid, new Date(dialogs.get(clientId)), endTime, experimentId);
                SensorData.saveSessionData(data, s);

                if (experimentId != null) {
                    Experiment.updateEndTime(experimentId, endTime, s);

                    if (reportExpEnd) {
                        EndpointInfoControl.broadcast(new ControlMessage(ControlMessage.Topic_DataLog,
                                ControlMessage.Command_DataLog_End).toString());
                    }
                }

            } catch (Exception e) {
                UtilLog.logWarn(EndpointDataLog.class, e);
            } finally {
                DatabaseHandler.closeSession(s);
            }
        }
        dialogFiles.remove(clientId);
        dialogs.remove(clientId);
        TimerCheckDataIdle.getCurrentTask().removeDialog(clientId);
        reportTotalDialogs();

        checkExperimentEnding();
    }

    public static int getTotalDialogs() {
        return dialogFiles.size();
    }

    public static int getTotalClients() {
        return clients.size();
    }

    private static void reportTotalDialogs() {
        int size = dialogFiles.size();
        UtilLog.logInfo(EndpointDataLog.class, "Total dialogs: " + size);
        try {
            EndpointInfoControl.broadcast(new ControlMessage(ControlMessage.Topic_Dialog_Count, String.valueOf(size)).toString());
            EndpointDrive.broadcast(new ControlMessage(ControlMessage.Topic_Refresh, null).toString());
        } catch (IOException ex) {
            UtilLog.logWarn(EndpointDataLog.class, ex);
        }
    }

    public static void dialogRemoved(String clientId) {
        endClientDialog(clientId, true);
    }

    @OnOpen
    public void onOpen(Session session) {
        session.setMaxIdleTimeout(1000 * 60 * 10);//0 not work in jetty
        UtilLog.logNetwork(this, "MaxIdleTimeout: " + session.getMaxIdleTimeout());

        clients.add(session);
        UtilLog.logNetwork(this, "Client joined: " + session.getId());
    }

    @OnClose
    public void onClose(Session session, CloseReason reason) throws IOException {
        UtilLog.logNetwork(this, "Client closed: " + session.getId() + ", due to " + reason);
        clients.remove(session);
        if (dialogs.containsKey(session.getId())) {
            endClientDialog(session.getId(), false);
        }
        RobotConnections.removeConnection(session.getId());
    }

    @OnError
    public void onError(Session session, Throwable error) {
        UtilLog.logError(this, "Client error: " + session.getId() + " " + error);
        clients.remove(session);
        if (dialogs.containsKey(session.getId())) {
            endClientDialog(session.getId(), false);
        }
        RobotConnections.removeConnection(session.getId());
    }

    @OnMessage
    public void onMessage(Session session, String message) {
        try {
            if (message.startsWith("{")) {
                UtilLog.logNetwork(this, "Got protocol message: " + message);
                processProtocol(ControlMessage.getMessage(message), session);
            } else {
                if (RobotConnections.getRobots().containsKey(session.getId())) {
                    processData(message, session);
                } else {
                    session.close(new CloseReason(CloseReason.CloseCodes.NORMAL_CLOSURE, "This WebSocket has not provided a Robot ID"));
                }
            }
        } catch (Exception e) {
            UtilLog.logWarn(this, e.toString());
        }
    }

    private void processData(String message, Session session) throws IOException {
        String checkedMessage = message.trim();
        if (checkedMessage.endsWith("\n")) {
            checkedMessage = checkedMessage.split("\n")[0];
        }
        EndpointLiveData.broadcast(message);

        if (pauseOnMessage.get()) {
            UtilLog.logWarn(this, "Doing critical process, so onMessage is discarded");
            return;
        }

        long currentTime = System.currentTimeMillis();

        if (!dialogFiles.containsKey(session.getId())) {
            //new client
            makeNewClientDialog(session.getId(), currentTime);
        }

        checkedMessage += " " + currentTime;
        checkedMessage += "\n";
        dialogFiles.get(session.getId()).write(checkedMessage);
//        long sessionPeriod = currentTime - dialogs.get(session.getId());
//        if (sessionPeriod > ServiceDialogLifeTime) {
//            //do not let 0 dialog end current experiment
//            String tempExperimentId = experimentId;
//
//            //dialog should be closed and make a new one
//            endClientDialog(session.getId(), false);
//
//            //revive the experiment
//            experimentId = tempExperimentId;
//            makeNewClientDialog(session.getId(), System.currentTimeMillis());
//        }
        TimerCheckDataIdle.getCurrentTask().updateDialog(session.getId(), currentTime);
    }

    private void processProtocol(ControlMessage m, Session session) throws IOException {
        if (m.getTopic().equals(ControlMessage.Topic_New_Connection)) {
            try {
                ArrayList content = (ArrayList) m.getContent();
                int id = ((Number) content.get(0)).intValue();
                RobotConnections.addConnection(session.getId(), new RobotInfo(id, session, (String) content.get(1)));
            } catch (Exception e) {
                session.close(new CloseReason(CloseReason.CloseCodes.NORMAL_CLOSURE, e.toString()));
            }
        } else {
            EndpointDrive.broadcast(m.toString());
        }
    }
}
