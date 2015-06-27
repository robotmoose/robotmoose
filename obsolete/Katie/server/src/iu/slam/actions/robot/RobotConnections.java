package iu.slam.actions.robot;

import iu.slam.models.RobotInfo;
import java.util.Map;
import java.util.concurrent.ConcurrentHashMap;

/**
 *
 * @author hanaldo
 */
public class RobotConnections {

    private static final Map<String, RobotInfo> robots = new ConcurrentHashMap<>(5);

    public static void addConnection(String connectionId, RobotInfo robot) throws Exception {
        RobotInfo r = getRobotById(robot.getRobotId());
        if (r != null) {
            throw new Exception("Same robot id is already online");
        }
        robots.put(connectionId, robot);
    }

    public static void removeConnection(String conntionId) {
        robots.remove(conntionId);
    }

    public static Map<String, RobotInfo> getRobots() {
        return robots;
    }

    public static RobotInfo getRobotById(int id) {
        for (RobotInfo r : robots.values()) {
            if (r.getRobotId() == id) {
                return r;
            }
        }
        return null;
    }

    public static int getRobotId(String connectionId) {
        RobotInfo r = robots.get(connectionId);
        if (r == null) {
            return -1;
        } else {
            return r.getRobotId();
        }
    }

    private RobotConnections() {
    }
}
