package iu.slam.actions.video;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Video;
import iu.slam.utils.UtilLog;
import java.util.List;
import org.apache.struts2.convention.annotation.Action;
import org.apache.struts2.convention.annotation.ParentPackage;
import org.apache.struts2.convention.annotation.Result;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
@ParentPackage("json-default")
public class ActionViewVideoList extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private int pageNumber, pageSize;
    private int totalItems;
    private List<Video> data;

    @Action(value = "/viewVideoList", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            data = Video.listVideo(s, pageNumber, pageSize);
            totalItems = Video.countRecords(s);

            return SUCCESS;
        } catch (Exception e) {
            UtilLog.logWarn(this, e);
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

    public int getTotalItems() {
        return totalItems;
    }

    public List<Video> getData() {
        return data;
    }

}
