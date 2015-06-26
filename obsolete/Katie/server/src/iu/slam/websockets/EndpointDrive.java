package iu.slam.websockets;

import iu.slam.actions.robot.RobotConnections;
import iu.slam.models.RobotInfo;
import iu.slam.protocol.ControlMessage;
import iu.slam.utils.UtilLog;
import java.io.IOException;
import java.util.Collections;
import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import javax.websocket.CloseReason;
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
@ServerEndpoint(value = "/driveKatie")
public class EndpointDrive {

    private static final Set<Session> clients = Collections.synchronizedSet(new HashSet<Session>(10));

    public static void broadcast(String message) throws IOException {
        //UtilLog.logInfo(EndpointDrive.class, "Broadcast: " + message);
        for (Session client : clients) {
            client.getBasicRemote().sendText(message);
        }
    }

    @OnOpen
    public void onOpen(Session session) {
        session.setMaxIdleTimeout(1000 * 60 * 10);//0 not work in jetty
        UtilLog.logNetwork(this, "MaxIdleTimeout: " + session.getMaxIdleTimeout());
        clients.add(session);
        UtilLog.logNetwork(this, "Client joined: " + session.getId());
        UtilLog.logNetwork(this, "Total: " + clients.size());
    }

    @OnClose
    public void onClose(Session session, CloseReason reason) throws IOException {
        clients.remove(session);
        UtilLog.logNetwork(this, "Client closed: " + session.getId() + ", due to " + reason);
    }

    @OnError
    public void onError(Session session, Throwable error) {
        clients.remove(session);
        UtilLog.logError(this, "Client error: " + session.getId() + " " + error);
    }

    @OnMessage
    public void onMessage(Session session, String message) {
        try {
            UtilLog.logNetwork(this, "onMessage: " + message);
            ControlMessage m = ControlMessage.getMessage(message);

            try {
                @SuppressWarnings("unchecked")
                Map<String, Object> r = (Map<String, Object>) m.getContent();
                int robotId = ((Number) r.get("robotId")).intValue();

                RobotInfo robot = RobotConnections.getRobotById(robotId);
                if (robot == null) {
                    throw new Exception("Robot " + robotId + " is not online");
                }
                robot.getConnection().getBasicRemote().sendText(message);
            } catch (Exception e) {
                UtilLog.logWarn(this, e.toString());
                m.setContent(false);
                session.getBasicRemote().sendText(m.toString());
            }
        } catch (Exception e) {
            UtilLog.logError(this, e.toString());
        }
    }
}
