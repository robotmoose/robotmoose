package iu.slam.actions.photo;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Photo;
import iu.slam.utils.UtilLog;
import java.util.List;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts2.convention.annotation.Action;
import org.apache.struts2.convention.annotation.ParentPackage;
import org.apache.struts2.convention.annotation.Result;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
@ParentPackage("json-default")
public class ActionShowPhoto extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private int pageNumber, pageSize, robotId;
    private int totalItems;
    private List<Photo> data;
    private String tag;

    @Action(value = "/showPhotos", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            if (tag == null) {
                data = Photo.listPhoto(s, pageNumber, pageSize, robotId);
                totalItems = Photo.countRecords(s, robotId);
            } else {
                data = Photo.listPhotoByTag(s, pageNumber, pageSize, robotId, tag);
                totalItems = Photo.countRecordsByTag(s, robotId, tag);
            }

            return SUCCESS;
        } catch (Exception e) {
            UtilLog.logWarn(this, e);
            getHttpResponse().sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, e.toString());
            return null;
        } finally {
            DatabaseHandler.closeSession(s);
        }
    }

    public void setPageNumber(int pageNumber) {
        this.pageNumber = pageNumber;
    }

    public void setPageSize(int pageSize) {
        this.pageSize = pageSize;
    }

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

    public int getTotalItems() {
        return totalItems;
    }

    public List<Photo> getData() {
        return data;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

}
