package iu.slam.actions.photo;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.PhotoDrawTag;
import iu.slam.utils.MyGson;
import iu.slam.utils.UtilLog;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

public class ActionAddPhotoDrawTag extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private String tagJson;

    @Action("/addDrawTag")
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            PhotoDrawTag.addTag(s, MyGson.getGSON().fromJson(tagJson, PhotoDrawTag.class));
            UtilLog.logInfo(this, "Tag added: " + tagJson);
        } catch (Exception e) {
            UtilLog.logError(this, e);
            getHttpResponse().sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, e.toString());
        } finally {
            DatabaseHandler.closeSession(s);
        }
        return null;
    }

    public void setTagJson(String tagJson) {
        this.tagJson = tagJson;
    }
}
