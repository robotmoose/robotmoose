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

/**
 *
 * @author hanaldo
 */
@Entity
@Table(name = "video")
public class Video implements Serializable {

    private static final long serialVersionUID = 1L;

    public static List<Video> listVideo(Session s, int pageNumber, int pageSize) {
        Criteria c = s.createCriteria(Video.class);
        c.addOrder(Order.desc("startTime"));
        int firstRecord = (pageNumber - 1) * pageSize;
        c.setFirstResult(firstRecord);
        c.setMaxResults(pageSize);
        @SuppressWarnings("unchecked")
        List<Video> data = c.list();
        return data;
    }

    public static int addVideo(Session s, Video v) {
        s.save(v);
        s.beginTransaction().commit();
        return v.getVid();
    }

    public static Video getVideo(Session s, int vid) {
        return (Video) s.get(Video.class, vid);
    }

    public static int countRecords(Session s) {
        Criteria c = s.createCriteria(Video.class);
        int total = ((Number) c.setProjection(Projections.rowCount()).uniqueResult()).intValue();
        return total;
    }

    private int vid;
    private String filePath, logId;
    private long videoLength;
    private Date startTime;

    public Video() {
    }

    public Video(String filePath, Date startTime) {
        this.filePath = filePath;
        this.startTime = startTime;
    }

    @Id
    @GeneratedValue
    @Column(name = "vid")
    public int getVid() {
        return vid;
    }

    public void setVid(int vid) {
        this.vid = vid;
    }

    @Column(name = "file_path")
    public String getFilePath() {
        return filePath;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

    @Column(name = "video_length")
    public long getVideoLength() {
        return videoLength;
    }

    public void setVideoLength(long videoLength) {
        this.videoLength = videoLength;
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

    @Column(name = "log_id")
    public String getLogId() {
        return logId;
    }

    public void setLogId(String logId) {
        this.logId = logId;
    }

}
