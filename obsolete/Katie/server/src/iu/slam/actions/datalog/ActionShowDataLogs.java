package iu.slam.actions.datalog;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.SensorData;
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
public class ActionShowDataLogs extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private int pageNumber, pageSize, robotId;
    private int totalItems;
    private List<SensorData> data;

    @Action(value = "/showDataLogs", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            data = SensorData.listData(s, pageNumber, pageSize, robotId);
            totalItems = SensorData.countRecords(s, robotId);

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

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

    public int getTotalItems() {
        return totalItems;
    }

    public List<SensorData> getData() {
        return data;
    }
}
