package iu.slam.utils;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

/**
 *
 * @author hanaldo
 */
public class UtilLog {

    private static final Log log = LogFactory.getLog(UtilLog.class);

    public static void logInfo(Object c, Object message) {
        log.info("<p style='color:#73e600;'>{" + getClassName(c) + "}:: " + message.toString() + "</p>");
    }

    public static void logDatabase(Object c, Object message) {
        log.info("<p style='color:#8a00ce;'>{" + getClassName(c) + "}:: " + message.toString() + "</p>");
    }

    public static void logEnvironment(Object c, Object message) {
        log.info("<p style='color:#919191;'>{" + getClassName(c) + "}:: " + message.toString() + "</p>");
    }

    public static void logNetwork(Object c, Object message) {
        log.info("<p style='color:#00d5ff;'>{" + getClassName(c) + "}:: " + message.toString() + "</p>");
    }

    public static void logWarn(Object c, Object message) {
        log.info("<p style='color:#ff00ff;'>{" + getClassName(c) + "}:: " + message.toString() + "</p>");
    }

    public static void logError(Object c, Object message) {
        log.info("<p style='color:#ff1b2d;'>{" + getClassName(c) + "}:: " + message.toString() + "</p>");
    }

    private static String getClassName(Object o) {
        if (o instanceof Class) {
            return o.toString();
        } else {
            return o.getClass().getName();
        }
    }

    private UtilLog() {
    }
}
