package iu.slam.models;

import java.util.Date;
import javax.websocket.Session;
import org.apache.struts2.json.annotations.JSON;

/**
 *
 * @author hanaldo
 */
public class RobotInfo {

    private int robotId;
    private Session connection;
    private Date activateTime;
    private String localIP;

    public RobotInfo(int robotId, Session connection, String localIP) {
        this.robotId = robotId;
        this.connection = connection;
        this.activateTime = new Date();
        this.localIP = localIP;
    }

    public int getRobotId() {
        return robotId;
    }

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

    @JSON(serialize = false, deserialize = false)
    public Session getConnection() {
        return connection;
    }

    public void setConnection(Session connection) {
        this.connection = connection;
    }

    public Date getActivateTime() {
        return activateTime;
    }

    public void setActivateTime(Date activateTime) {
        this.activateTime = activateTime;
    }

    public String getLocalIP() {
        return localIP;
    }

    public void setLocalIP(String localIP) {
        this.localIP = localIP;
    }

}
