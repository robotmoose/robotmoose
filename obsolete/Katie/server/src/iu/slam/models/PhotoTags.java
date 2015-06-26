package iu.slam.models;

import java.io.Serializable;
import java.util.List;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Id;
import javax.persistence.Table;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;

/**
 *
 * @author hanaldo
 */
@Entity
@Table(name = "photo_tags")
public class PhotoTags implements Serializable {

    private static final long serialVersionUID = 1L;

    public static void addTag(Session s, int photoId, String tag) {
        s.save(new PhotoTags(photoId, tag));
        s.beginTransaction().commit();
    }

    public static void removeTag(Session s, int photoId, String tag) {
        Criteria c = s.createCriteria(PhotoTags.class);
        c.add(Restrictions.eq("photoId", photoId))
                .add(Restrictions.eq("tag", tag));
        @SuppressWarnings("unchecked")
        List<PhotoTags> tags = c.list();
        for (PhotoTags t : tags) {
            s.delete(t);
        }
        s.beginTransaction().commit();
    }

    @SuppressWarnings("unchecked")
    public static List<String> listAllTag(Session s) {
        Criteria c = s.createCriteria(PhotoTags.class);
        c.setProjection(Projections.distinct(Projections.property("tag")));
        return c.list();
    }

    @SuppressWarnings({"unchecked"})
    public static List<String> getPhotoTags(Session s, int id) {
        Criteria c = s.createCriteria(PhotoTags.class);
        c.add(Restrictions.eq("photoId", id));
        c.setProjection(Projections.property("tag"));
        return c.list();
    }

    private int photoId;
    private String tag;

    public PhotoTags() {
    }

    public PhotoTags(int photoId, String tag) {
        this.photoId = photoId;
        this.tag = tag;
    }

    @Id
    @Column(name = "photo_id")
    public int getPhotoId() {
        return photoId;
    }

    public void setPhotoId(int photoId) {
        this.photoId = photoId;
    }

    @Id
    @Column(name = "tag")
    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }
}
