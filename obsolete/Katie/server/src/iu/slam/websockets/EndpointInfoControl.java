package iu.slam.websockets;

import iu.slam.protocol.ControlMessage;
import iu.slam.utils.UtilLog;
import java.io.IOException;
import java.util.Collections;
import java.util.HashSet;
import java.util.Set;
import javax.websocket.CloseReason;
import javax.websocket.EncodeException;
import javax.websocket.OnClose;
import javax.websocket.OnError;
import javax.websocket.OnMessage;
import javax.websocket.OnOpen;
import javax.websocket.Session;
import javax.websocket.server.ServerEndpoint;

/**
 *
 * @author hanaldo
 */
@ServerEndpoint(value = "/infoControl")
public class EndpointInfoControl {

    private static final Set<Session> clients = Collections.synchronizedSet(new HashSet<Session>(10));

    public static void broadcast(String message) throws IOException {
        //UtilLog.logInfo(EndpointInfoControl.class, "Broadcast: " + message);
        for (Session client : clients) {
            client.getBasicRemote().sendText(message);
        }
    }

    @OnOpen
    public void onOpen(Session session) throws IOException {
        session.setMaxIdleTimeout(1000 * 60 * 10);//0 not work in jetty
        clients.add(session);
        UtilLog.logInfo(this, "Client joined: " + session.getId());
        UtilLog.logInfo(this, "Total: " + clients.size());

        session.getBasicRemote().sendText(
                new ControlMessage(ControlMessage.Topic_Dialog_Count, String.valueOf(EndpointDataLog.getTotalDialogs())).toString());
    }

    @OnClose
    public void onClose(Session session, CloseReason reason) throws IOException {
        clients.remove(session);
        UtilLog.logInfo(this, "Client closed: " + session.getId() + ", due to " + reason);
    }

    @OnError
    public void onError(Session session, Throwable error) {
        clients.remove(session);
        UtilLog.logInfo(this, "Client error: " + session.getId() + " " + error);
    }

    @OnMessage
    public void onMessage(Session session, String message) throws IOException, EncodeException {
        UtilLog.logInfo(this, "onMessage: " + message);
        ControlMessage command = ControlMessage.getMessage(message);
        switch (command.getTopic()) {
            case ControlMessage.Topic_DataLog:
                switch ((String) command.getCommand()) {
                    case ControlMessage.Command_DataLog_Start:
                        boolean ok = EndpointDataLog.startRecording((String) command.getContent());
                        if (ok) {
                            command.setContent(new String[]{EndpointDataLog.getExperimentId(), EndpointDataLog.getExperimentName()});
                            broadcast(command.toString());
                        } else {
                            command.setCommand(ControlMessage.Command_DataLog_End);
                            broadcast(command.toString());
                        }
                        break;
                    case ControlMessage.Command_DataLog_End:
                        EndpointDataLog.endAllDialogs();
                        broadcast(command.toString());
                        break;
                    case ControlMessage.Command_DataLog_Check:
                        if (EndpointDataLog.isRecordingStarted()) {
                            command.setCommand(ControlMessage.Command_DataLog_Start);
                            command.setContent(new String[]{EndpointDataLog.getExperimentId(), EndpointDataLog.getExperimentName()});
                            session.getBasicRemote().sendText(command.toString());
                        } else {
                            command.setCommand(ControlMessage.Command_DataLog_End);
                            session.getBasicRemote().sendText(command.toString());
                        }
                        break;
                }
                break;
        }
    }
}
