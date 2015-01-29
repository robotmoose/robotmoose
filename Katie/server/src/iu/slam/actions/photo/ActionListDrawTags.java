package iu.slam.actions.photo;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.PhotoDrawTag;
import iu.slam.utils.UtilLog;
import java.util.List;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts2.convention.annotation.Action;
import org.apache.struts2.convention.annotation.ParentPackage;
import org.apache.struts2.convention.annotation.Result;
import org.hibernate.Session;

@ParentPackage("json-default")
public class ActionListDrawTags extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private List<PhotoDrawTag> tags;
    private int photo;

    @Action(value = "/listDrawTag", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            tags = PhotoDrawTag.listAllTag(s, photo);
            UtilLog.logInfo(this, "listDrawTag");
            return SUCCESS;
        } catch (Exception e) {
            UtilLog.logError(this, e);
            getHttpResponse().sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, e.toString());
            return null;
        } finally {
            DatabaseHandler.closeSession(s);
        }
    }

    public List<PhotoDrawTag> getTags() {
        return tags;
    }

    public void setPhoto(int photo) {
        this.photo = photo;
    }
}
