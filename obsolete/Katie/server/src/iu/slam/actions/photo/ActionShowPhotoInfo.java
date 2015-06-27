package iu.slam.actions.photo;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Photo;
import iu.slam.models.PhotoTags;
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
public class ActionShowPhotoInfo extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private int photoId;
    private Photo photo;
    private List<String> tags;

    @Action(value = "/getPhotoInfo", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            photo = Photo.getPhoto(s, photoId);
            tags = PhotoTags.getPhotoTags(s, photoId);
            return SUCCESS;
        } catch (Exception e) {
            UtilLog.logError(this, e);
            return null;
        } finally {
            DatabaseHandler.closeSession(s);
        }
    }

    public void setPhotoId(int photoId) {
        this.photoId = photoId;
    }

    public Photo getPhoto() {
        return photo;
    }

    public List<String> getTags() {
        return tags;
    }
}
