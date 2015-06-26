package iu.slam.websockets;

import iu.slam.utils.UtilLog;
import java.util.HashMap;
import java.util.Map;
import java.util.TimerTask;

/**
 *
 * @author hanaldo
 */
public class TaskCheckDataIdle extends TimerTask {

    private Map<String, Long> dialogLasts;

    public TaskCheckDataIdle() {
        dialogLasts = new HashMap<>(10);
    }

    public synchronized void addDialog(String clientId) {
        dialogLasts.put(clientId, System.currentTimeMillis());
    }

    public synchronized void updateDialog(String clientId, long now) {
        dialogLasts.put(clientId, now);
    }

    public synchronized void removeDialog(String clientId) {
        dialogLasts.remove(clientId);
    }

    @Override
    public synchronized void run() {
        for (String client : dialogLasts.keySet()) {
            Long last = dialogLasts.get(client);
            long diff = System.currentTimeMillis() - last;
            if (diff > 1000 * 5) {
                removeDialog(client);
                EndpointDataLog.dialogRemoved(client);
                UtilLog.logInfo(this, "The dialog of client " + client + " is timedout");
            }
        }
    }
}
