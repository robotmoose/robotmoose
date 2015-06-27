package iu.slam.models;

import java.io.Serializable;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.Table;
import javax.persistence.Temporal;
import javax.persistence.Transient;
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
@Table(name = "photo")
public class Photo implements Serializable {

    private static final long serialVersionUID = 1L;

    public static void addPhoto(Photo photo, Session session) {
        session.save(photo);
        session.beginTransaction().commit();
    }

    public static Photo getPhoto(Session s, int id) {
        return (Photo) s.get(Photo.class, id);
    }

    public static List<Photo> listPhoto(Session session, int pageNumber, int pageSize, int robotId) {
        Criteria c = session.createCriteria(Photo.class);
        c.add(Restrictions.eq("robotId", robotId));
        c.addOrder(Order.desc("saveTime"));
        int firstRecord = (pageNumber - 1) * pageSize;
        c.setFirstResult(firstRecord);
        c.setMaxResults(pageSize);
        @SuppressWarnings("unchecked")
        List<Photo> data = c.list();
        return data;
    }

    public static List<Photo> listPhotoByTag(Session session, int pageNumber, int pageSize, int robotId, String tag) {
        Criteria c = session.createCriteria(PhotoTags.class)
                .add(Restrictions.eq("tag", tag))
                .setProjection(Projections.distinct(Projections.property("photoId")));
        List<Integer> photoIds = c.list();

        c = session.createCriteria(PhotoDrawTag.class)
                .add(Restrictions.eq("tag", tag))
                .setProjection(Projections.distinct(Projections.property("photo")));
        photoIds.addAll(c.list());

        if (photoIds.isEmpty()) {
            return new ArrayList<>(0);
        }

        c = session.createCriteria(Photo.class);
        c.add(Restrictions.eq("robotId", robotId));
        c.add(Restrictions.in("id", photoIds));
        c.addOrder(Order.desc("saveTime"));
        int firstRecord = (pageNumber - 1) * pageSize;
        c.setFirstResult(firstRecord);
        c.setMaxResults(pageSize);
        @SuppressWarnings("unchecked")
        List<Photo> data = c.list();
        return data;
    }

    public static int countRecords(Session session, int robotId) {
        Criteria c = session.createCriteria(Photo.class);
        c.add(Restrictions.eq("robotId", robotId));
        int total = ((Number) c.setProjection(Projections.rowCount()).uniqueResult()).intValue();
        return total;
    }

    public static int countRecordsByTag(Session session, int robotId, String tag) {
        Criteria c = session.createCriteria(PhotoTags.class);
        c.add(Restrictions.eq("tag", tag));
        c.setProjection(Projections.distinct(Projections.property("photoId")));
        @SuppressWarnings("unchecked")
        List<Integer> photoIds = c.list();
        if (photoIds.isEmpty()) {
            return 0;
        }

        c = session.createCriteria(Photo.class);
        c.add(Restrictions.eq("robotId", robotId));
        c.add(Restrictions.in("id", photoIds));
        int total = ((Number) c.setProjection(Projections.rowCount()).uniqueResult()).intValue();
        return total;
    }

    private int id, robotId;
    private String name, filePath, experimentId;
    private Date saveTime;

    public Photo() {
    }

    public Photo(int robotId, String name, String filePath, String experimentId) {
        this.robotId = robotId;
        this.name = name;
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

    @Column(name = "name")
    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    @Column(name = "file_path")
    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    @Column(name = "save_time")
    @Temporal(javax.persistence.TemporalType.TIMESTAMP)
    @JSON(format = "yyyy-MMM-dd HH:mm:ss")
    public Date getSaveTime() {
        return saveTime;
    }

    public void setSaveTime(Date saveTime) {
        this.saveTime = saveTime;
    }

    @Transient
    public long getSaveTimeValue() {
        return saveTime.getTime();
    }

    @Column(name = "robot_id")
    public int getRobotId() {
        return robotId;
    }

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

    @Column(name = "experiment_id")
    public String getExperimentId() {
        return experimentId;
    }

    public void setExperimentId(String experimentId) {
        this.experimentId = experimentId;
    }

}
