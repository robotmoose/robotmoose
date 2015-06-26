package iu.slam.actions.video;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Video;
import iu.slam.utils.HttpUtil;
import iu.slam.utils.MyContextListener;
import iu.slam.utils.UtilLog;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Date;
import java.util.concurrent.atomic.AtomicBoolean;
import javax.servlet.http.HttpServletResponse;
import org.apache.http.Header;
import org.apache.http.HttpStatus;
import org.apache.http.auth.UsernamePasswordCredentials;
import org.apache.http.impl.auth.BasicScheme;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
public class ActionRecordVideo extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    public static final AtomicBoolean start = new AtomicBoolean(false);
    private static int recordId = -1;

    public static void copyStream(InputStream in, OutputStream out) throws IOException {
        try {
            byte[] buffer = new byte[1024 * 4];
            int n = 0;
            while (-1 != (n = in.read(buffer))) {
                out.write(buffer, 0, n);
                if (!start.get()) {
                    out.close();
                    in.close();
                    return;
                }
            }
        } finally {
            out.close();
            in.close();
        }
    }
    private String ip, controll;

    @Action("/record")
    @Override
    public String execute() throws Exception {
        if (ip == null || controll == null) {
            getHttpResponse().sendError(HttpStatus.SC_BAD_REQUEST);
            return null;
        }
        if (ip.isEmpty() || controll.isEmpty()) {
            getHttpResponse().sendError(HttpStatus.SC_BAD_REQUEST);
            return null;
        }

        switch (controll) {
            case "start":
                startRecording(ip, getHttpResponse());
                break;
            case "end":
                endRecording();
                break;
        }
        return null;
    }

    private synchronized void startRecording(final String ip, HttpServletResponse response) throws IOException {
        if (start.get()) {
            UtilLog.logInfo(this, "Already started!");
            return;
        }
        Session s = DatabaseHandler.getSession();
        try {
            Header authHeader = BasicScheme.authenticate(
                    new UsernamePasswordCredentials("test", "test"),
                    "UTF-8", false);
            final InputStream in = HttpUtil.httpGetStream("http://" + ip + ":80/axis-cgi/mjpg/video.cgi", new Header[]{authHeader});

            Date now = new Date();

            String filename = "video/v-" + now.getTime() + ".mjpeg";
            String localPath = MyContextListener.getContextPath() + filename;

            final FileOutputStream out = new FileOutputStream(localPath);
            Video video = new Video(filename, now);
            recordId = Video.addVideo(s, video);

            UtilLog.logInfo(this, "Start recording video{" + recordId + "} stream...");
            start.set(true);
            new Thread() {
                @Override
                public void run() {
                    try {
                        copyStream(in, out);
                        UtilLog.logInfo(this, "Finished");
                    } catch (IOException ex) {
                        UtilLog.logError(this, ex);
                    }
                }
            }.start();
        } catch (Exception ex) {
            UtilLog.logError(this, ex);
            if (!response.isCommitted()) {
                response.sendError(HttpStatus.SC_BAD_REQUEST, ex.toString());
            }
        } finally {
            DatabaseHandler.closeSession(s);
        }

    }

    private synchronized void endRecording() {
        start.set(false);
        Session s = DatabaseHandler.getSession();
        try {
            Video video = Video.getVideo(s, recordId);
            video.setVideoLength((System.currentTimeMillis() - video.getStartTime().getTime()) / 1000);
            s.beginTransaction().commit();
        } finally {
            DatabaseHandler.closeSession(s);
        }
        recordId = -1;
    }

    public void setIp(String ip) {
        this.ip = ip;
    }

    public void setControll(String controll) {
        this.controll = controll;
    }
}
