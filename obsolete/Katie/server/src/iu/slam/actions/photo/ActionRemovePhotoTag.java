package iu.slam.actions.photo;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.PhotoTags;
import iu.slam.utils.UtilLog;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
public class ActionRemovePhotoTag extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private String tag;
    private int photoId;

    @Action("/removeTag")
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            PhotoTags.removeTag(s, photoId, tag);
            UtilLog.logInfo(this, "Tag removed: " + tag + " " + photoId);
        } catch (Exception e) {
            UtilLog.logError(this, e);
            getHttpResponse().sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, e.toString());
        } finally {
            DatabaseHandler.closeSession(s);
        }
        return null;
    }

    public void setTag(String tag) {
        this.tag = tag;
    }

    public void setPhotoId(int photoId) {
        this.photoId = photoId;
    }

}
