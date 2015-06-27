package iu.slam.websockets;

import iu.slam.utils.UtilLog;
import java.io.IOException;
import java.util.Timer;
import java.util.TimerTask;

/**
 *
 * @author hanaldo
 */
public class TimerForceHeartBeat {
    
    private static Timer timer;
    
    public static void clean() {
        if (timer != null) {
            try {
                timer.cancel();
            } finally {
                timer = null;
                UtilLog.logInfo(TimerForceHeartBeat.class, "TimerForceHeartBeat stopped");
            }
        }
    }
    
    public static void start() {
        if (timer != null) {
            clean();
        }
        timer = new Timer();
        timer.scheduleAtFixedRate(new TimerTask() {
            
            @Override
            public void run() {
                try {
                    EndpointDataLog.broadcast("{\"hb\":\"hb\"}");
                    EndpointDrive.broadcast("{\"hb\":\"hb\"}");
                    EndpointInfoControl.broadcast("{\"hb\":\"hb\"}");
                } catch (IOException ex) {
                    UtilLog.logWarn(TimerForceHeartBeat.class, ex);
                }
            }
        }, 10000, 1000 * 60 * 2);
        UtilLog.logInfo(TimerForceHeartBeat.class, "TimerForceHeartBeat started");
    }
    
    private TimerForceHeartBeat() {
    }
}
