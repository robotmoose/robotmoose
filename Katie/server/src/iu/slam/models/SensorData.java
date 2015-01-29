package iu.slam.models;

import java.io.Serializable;
import java.util.Date;
import java.util.List;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.persistence.Temporal;
import org.apache.struts2.json.annotations.JSON;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Order;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;

/**
 *
 * @author hanaldo
 */
@Entity
@Table(name = "alldata")
public class SensorData implements Serializable {

    private static final long serialVersionUID = 1L;

    public static void saveSessionData(SensorData data, Session session) {
        session.save(data);
        session.beginTransaction().commit();
    }

    public static List<SensorData> listData(Session session, int pageNumber, int pageSize, int robotId) {
        Criteria c = session.createCriteria(SensorData.class);
        c.add(Restrictions.eq("robotId", robotId));
        c.addOrder(Order.desc("startTime"));
        int firstRecord = (pageNumber - 1) * pageSize;
        c.setFirstResult(firstRecord);
        c.setMaxResults(pageSize);
        @SuppressWarnings("unchecked")
        List<SensorData> data = c.list();
        return data;
    }

    public static int countRecords(Session session, int robotId) {
        Criteria c = session.createCriteria(SensorData.class);
        c.add(Restrictions.eq("robotId", robotId));
        int total = ((Number) c.setProjection(Projections.rowCount()).uniqueResult()).intValue();
        return total;
    }

    public static List<SensorData> listExperimentData(String experimentId, Session session) {
        Criteria c = session.createCriteria(SensorData.class);
        c.add(Restrictions.eq("experimentId", experimentId));
        c.addOrder(Order.desc("startTime"));
        @SuppressWarnings("unchecked")
        List<SensorData> data = c.list();
        return data;
    }
    private int id, robotId;
    private Date startTime, endTime;
    private String filePath, experimentId;

    public SensorData() {
    }

    public SensorData(String filePath, int robotId, Date startTime, Date endTime, String experimentId) {
        this.robotId = robotId;
        this.startTime = startTime;
        this.endTime = endTime;
        this.filePath = filePath;
        this.experimentId = experimentId;
    }

    @Id
    @GeneratedValue
    @Column(name = "id")
    public int getId() {
        return id;
    }

    public void setId(int id) {
        this.id = id;
    }

    @Column(name = "file_path")
    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    @Column(name = "robot_id")
    public int getRobotId() {
        return robotId;
    }

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

    @Column(name = "start_time")
    @Temporal(javax.persistence.TemporalType.TIMESTAMP)
    @JSON(format = "MMM dd yyyy, hh:mm:ss a")
    public Date getStartTime() {
        return startTime;
    }

    public void setStartTime(Date startTime) {
        this.startTime = startTime;
    }

    @Column(name = "end_time")
    @Temporal(javax.persistence.TemporalType.TIMESTAMP)
    @JSON(format = "MMM dd yyyy, hh:mm:ss a")
    public Date getEndTime() {
        return endTime;
    }

    public void setEndTime(Date endTime) {
        this.endTime = endTime;
    }

    @Column(name = "experiment_id")
    public String getExperimentId() {
        return experimentId;
    }

    public void setExperimentId(String experimentId) {
        this.experimentId = experimentId;
    }
}
