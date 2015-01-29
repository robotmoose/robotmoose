package iu.slam.models;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import javax.persistence.Column;
import javax.persistence.Entity;
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
@Table(name = "experiment")
public class Experiment implements Serializable {

    private static final long serialVersionUID = 1L;

    public static List<Experiment> listExperiments(Session session, int pageNumber, int pageSize, int robotId) {
        Criteria c = session.createCriteria(SensorData.class);
        c.add(Restrictions.eq("robotId", robotId));
        c.add(Restrictions.isNotNull("experimentId"));
        c.setProjection(Projections.property("experimentId"));
        @SuppressWarnings("unchecked")
        List<String> experimentIds = c.list();
        if (experimentIds.isEmpty()) {
            return new ArrayList<>(0);
        }

        c = session.createCriteria(Experiment.class);
        c.add(Restrictions.eq("valid", true));
        c.addOrder(Order.desc("startTime"));
        c.add(Restrictions.in("id", experimentIds));
        int firstRecord = (pageNumber - 1) * pageSize;
        c.setFirstResult(firstRecord);
        c.setMaxResults(pageSize);
        @SuppressWarnings("unchecked")
        List<Experiment> experiments = c.list();
        return experiments;
    }

    public static int countRecords(Session session, int robotId) {
        Criteria c = session.createCriteria(SensorData.class);
        c.add(Restrictions.eq("robotId", robotId));
        c.add(Restrictions.isNotNull("experimentId"));
        c.setProjection(Projections.property("experimentId"));
        @SuppressWarnings("unchecked")
        List<String> experimentIds = c.list();
        if (experimentIds.isEmpty()) {
            return 0;
        }

        c = session.createCriteria(Experiment.class);
        c.add(Restrictions.eq("valid", true));
        c.add(Restrictions.in("id", experimentIds));
        int total = ((Number) c.setProjection(Projections.rowCount()).uniqueResult()).intValue();
        return total;
    }

    public static void addNewExperiment(String id, Date startTime, String name, Session session) {
        Experiment record = new Experiment();
        record.setId(id);
        record.setStartTime(startTime);
        record.setNote(name);
        session.save(record);
        session.beginTransaction().commit();
    }

    public static void updateNote(String id, String note, Session session) {
        Experiment record = (Experiment) session.get(Experiment.class, id);
        record.setNote(note);
        session.update(record);
        session.beginTransaction().commit();
    }

    public static String getNote(String id, Session session) {
        Experiment record = (Experiment) session.get(Experiment.class, id);
        return record.getNote();
    }

    public static void updateEndTime(String id, Date endTime, Session session) {
        Experiment record = (Experiment) session.get(Experiment.class, id);
        record.setEndTime(endTime);
        record.setValid(true);
        session.update(record);
        session.beginTransaction().commit();
    }

    public static boolean isExperimentStarted(String id, Session session) {
        Experiment record = (Experiment) session.get(Experiment.class, id);
        return record != null;
    }
    private String id, note;
    private Date startTime, endTime;
    private boolean valid;

    public Experiment() {
    }

    @Id
    @Column(name = "id")
    public String getId() {
        return id;
    }

    public void setId(String id) {
        this.id = id;
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

    @Column(name = "note")
    public String getNote() {
        return note;
    }

    public void setNote(String note) {
        this.note = note;
    }

    @Column(name = "valid")
    public boolean isValid() {
        return valid;
    }

    public void setValid(boolean valid) {
        this.valid = valid;
    }
}
