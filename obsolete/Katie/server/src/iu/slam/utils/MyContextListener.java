package iu.slam.utils;

import iu.slam.database.DatabaseHandler;
import iu.slam.websockets.TimerCheckDataIdle;
import iu.slam.websockets.TimerForceHeartBeat;
import javax.servlet.ServletContextEvent;
import javax.servlet.ServletContextListener;

/**
 *
 * @author hanaldo
 */
public class MyContextListener implements ServletContextListener {

    private static String contextPath;

    public static String getContextPath() {
        return contextPath;
    }

    @Override
    public void contextInitialized(ServletContextEvent sce) {
        contextPath = sce.getServletContext().getRealPath("/");
        if (!contextPath.endsWith("/")) {
            contextPath += "/";
        }
        UtilLog.logEnvironment(this, "Servlet Context Path: " + contextPath);

        MyGson.getGSON();
        DatabaseHandler.getInstance();
        TimerCheckDataIdle.start();
        TimerForceHeartBeat.start();

        UtilLog.logEnvironment(this, "contextInitialized");
    }

    @Override
    public void contextDestroyed(ServletContextEvent sce) {
        TimerCheckDataIdle.clean();
        TimerForceHeartBeat.clean();
        MyGson.clean();
        DatabaseHandler.getInstance().clean();
        UtilLog.logEnvironment(this, "contextDestroyed");
    }
}
