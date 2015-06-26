package iu.slam.actions.photo;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Photo;
import iu.slam.protocol.ControlMessage;
import iu.slam.utils.MyContextListener;
import iu.slam.utils.UtilLog;
import iu.slam.websockets.EndpointDrive;
import java.io.File;
import org.apache.commons.io.FileUtils;
import org.apache.commons.lang3.time.DateFormatUtils;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
public class ActionUploadPhoto extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private File image;
    private int robotId;

    @Action("/uploadPhoto")
    @Override
    public String execute() throws Exception {
        if (image != null) {
            UtilLog.logInfo(this, "Receiving photo");

            long time = System.currentTimeMillis();
            String timeString = Long.toString(time);
            String name = DateFormatUtils.format(time, "yyyy-MM-dd HH:mm:ss");

            String contextPath = MyContextListener.getContextPath();
            String localPath = contextPath + "photos/" + timeString + ".jpg";
            File dest = new File(localPath);

            FileUtils.copyFile(image, dest);
            UtilLog.logInfo(this, "file saved to: " + localPath);

            Session session = DatabaseHandler.getSession();
            try {
                Photo.addPhoto(new Photo(robotId, name, "photos/" + timeString + ".jpg", "exp"), session);
                EndpointDrive.broadcast(new ControlMessage(ControlMessage.Topic_Refresh, null).toString());
            } catch (Exception e) {
                UtilLog.logError(this, e);
            } finally {
                session.close();
            }

        }
        return null;
    }

    public void setImage(File image) {
        this.image = image;
    }

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

}
