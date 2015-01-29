package iu.slam.actions.photo;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.PhotoDrawTag;
import iu.slam.models.PhotoTags;
import iu.slam.utils.UtilLog;
import java.util.List;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts2.convention.annotation.Action;
import org.apache.struts2.convention.annotation.ParentPackage;
import org.apache.struts2.convention.annotation.Result;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
@ParentPackage("json-default")
public class ActionListAllPhotoTags extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private List<String> tags;

    @Action(value = "/listAllTag", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            tags = PhotoTags.listAllTag(s);
            tags.addAll(PhotoDrawTag.listAllTagString(s));
            UtilLog.logInfo(this, "listAllTag");
            return SUCCESS;
        } catch (Exception e) {
            UtilLog.logError(this, e);
            getHttpResponse().sendError(HttpServletResponse.SC_INTERNAL_SERVER_ERROR, e.toString());
            return null;
        } finally {
            DatabaseHandler.closeSession(s);
        }
    }

    public List<String> getTags() {
        return tags;
    }
}
