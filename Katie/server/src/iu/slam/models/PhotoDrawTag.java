package iu.slam.models;

import java.io.Serializable;
import java.util.List;
import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;
import javax.persistence.Table;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Projections;
import org.hibernate.criterion.Restrictions;

@Entity
@Table(name = "photo_draw_tags")
public class PhotoDrawTag implements Serializable {

    private static final long serialVersionUID = 1L;

    public static void addTag(Session s, PhotoDrawTag tag) {
        s.save(tag);
        s.beginTransaction().commit();
    }

    public static void removeTag(Session s, int photo, String tag) {
        Criteria c = s.createCriteria(PhotoDrawTag.class);
        c.add(Restrictions.eq("photo", photo))
                .add(Restrictions.eq("tag", tag));
        @SuppressWarnings("unchecked")
        List<PhotoDrawTag> tags = c.list();
        for (PhotoDrawTag t : tags) {
            s.delete(t);
        }
        s.beginTransaction().commit();
    }

    @SuppressWarnings("unchecked")
    public static List<PhotoDrawTag> listAllTag(Session s, int photo) {
        Criteria c = s.createCriteria(PhotoDrawTag.class)
                .add(Restrictions.eq("photo", photo));
        return c.list();
    }

    @SuppressWarnings("unchecked")
    public static List<String> listAllTagString(Session s) {
        Criteria c = s.createCriteria(PhotoDrawTag.class);
        c.setProjection(Projections.distinct(Projections.property("tag")));
        return c.list();
    }

    private int id, photo, x, y, width, height;
    private String tag;

    public PhotoDrawTag() {
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

    @Column(name = "photo")
    public int getPhoto() {
        return photo;
    }

    public void setPhoto(int photo) {
        this.photo = photo;
    }

    @Column(name = "x")
    public int getX() {
        return x;
    }

    public void setX(int x) {
        this.x = x;
    }

    @Column(name = "y")
    public int getY() {
        return y;
    }

    public void setY(int y) {
        this.y = y;
    }

    @Column(name = "width")
    public int getWidth() {
        return width;
    }

    public void setWidth(int width) {
        this.width = width;
    }

    @Column(name = "height")
    public int getHeight() {
        return height;
    }

    public void setHeight(int height) {
        this.height = height;
    }

    @Column(name = "tag")
    public String getTag() {
        return tag;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

}
