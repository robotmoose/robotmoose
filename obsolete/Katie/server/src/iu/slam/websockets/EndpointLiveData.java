package iu.slam.websockets;

import iu.slam.utils.UtilLog;
import static iu.slam.websockets.EndpointLiveData.Endpoint_Name;
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
@ServerEndpoint(value = Endpoint_Name)
public class EndpointLiveData {

    public static final String Endpoint_Name = "/liveData";
    private static final Set<Session> clients = Collections.synchronizedSet(new HashSet<Session>(10));

    public static void broadcast(String message) throws IOException {
        for (Session client : clients) {
            client.getBasicRemote().sendText(message);
        }
    }

    @OnOpen
    public void onOpen(Session session) {
        clients.add(session);
        UtilLog.logInfo(this, Endpoint_Name + " client joined: " + session.getId());
        UtilLog.logInfo(this, "total: " + clients.size());
    }

    @OnClose
    public void onClose(Session session, CloseReason reason) throws IOException {
        clients.remove(session);
        UtilLog.logInfo(this, Endpoint_Name + " client closed: " + session.getId() + ", due to " + reason);
    }

    @OnError
    public void onError(Session session, Throwable error) {
        clients.remove(session);
        UtilLog.logInfo(this, Endpoint_Name + " client error: " + session.getId() + " " + error);
    }

    @OnMessage
    public void onMessage(Session session, String message) throws IOException, EncodeException {
        UtilLog.logInfo(this, Endpoint_Name + " discard onMessage: " + message);
    }
}
