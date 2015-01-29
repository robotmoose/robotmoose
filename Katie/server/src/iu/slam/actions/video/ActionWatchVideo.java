package iu.slam.actions.video;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Video;
import iu.slam.utils.MyContextListener;
import iu.slam.utils.UtilLog;
import java.io.FileInputStream;
import javax.servlet.http.HttpServletResponse;
import org.apache.http.HttpStatus;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
public class ActionWatchVideo extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    public static final String CRLF = "\r\n";
    private int vid;
    private int delay;

    @Action("/watch")
    @Override
    public String execute() throws Exception {
        if (vid <= 0 || delay <= 8) {
            getHttpResponse().sendError(HttpStatus.SC_BAD_REQUEST);
            return null;
        }

        HttpServletResponse response = getHttpResponse();
        response.setHeader("Cache-Control", "no-cache");
        response.setHeader("Pragma", "no-cache");
        response.setHeader("Connection", "close");
        response.setHeader("Content-Type", "multipart/x-mixed-replace; boundary=--myboundary");

        Session s = DatabaseHandler.getSession();
        String localPath = null;
        try {
            localPath = MyContextListener.getContextPath() + Video.getVideo(s, vid).getFilePath();
        } catch (Exception ex) {
            UtilLog.logError(this, ex);
            getHttpResponse().sendError(HttpStatus.SC_INTERNAL_SERVER_ERROR, ex.toString());
        } finally {
            DatabaseHandler.closeSession(s);
        }

        try (FileInputStream file = new FileInputStream(localPath)) {
            byte[] buffer = new byte[4096];
            int bytesRead;
            long d = delay;
            while ((bytesRead = file.read(buffer)) != -1) {
                response.getOutputStream().write(buffer, 0, bytesRead);
                Thread.sleep(d);
            }
        } catch (Exception ex) {
            UtilLog.logError(this, ex);
            getHttpResponse().sendError(HttpStatus.SC_INTERNAL_SERVER_ERROR, ex.toString());
        }

        UtilLog.logInfo(this, "Video file sending finished");
        return null;
    }

    public void setVid(int vid) {
        this.vid = vid;
    }

    public void setDelay(int delay) {
        this.delay = delay;
    }

}
