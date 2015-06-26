package iu.slam.websockets;

import iu.slam.utils.UtilLog;
import java.util.Timer;

/**
 *
 * @author hanaldo
 */
public class TimerCheckDataIdle {

    private static Timer timer;
    private static TaskCheckDataIdle currentTask;

    public static void clean() {
        if (timer != null) {
            try {
                timer.cancel();
            } finally {
                timer = null;
                UtilLog.logInfo(TimerCheckDataIdle.class, "TimerCheckDataIdle stopped");
            }
        }
    }

    public static TaskCheckDataIdle getCurrentTask() {
        return currentTask;
    }

    public static void start() {
        if (timer != null) {
            clean();
        }
        timer = new Timer();
        currentTask = new TaskCheckDataIdle();
        timer.scheduleAtFixedRate(currentTask, 10000, 1000 * 6);
        UtilLog.logInfo(TimerCheckDataIdle.class, "TimerCheckDataIdle started");
    }

    private TimerCheckDataIdle() {
    }
}
