package iu.slam.actions.datalog;

import iu.slam.interfaces.AbstractMyAction;
import iu.slam.utils.MyContextListener;
import java.io.File;
import javax.servlet.http.HttpServletResponse;
import org.apache.commons.io.FileUtils;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.struts2.convention.annotation.Action;

/**
 *
 * @author hanaldo
 */
public class ActionGetFile extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private static final Log log = LogFactory.getLog(ActionGetFile.class);

    private String filePath;

    @Action("/getFile")
    @Override
    public String execute() throws Exception {
        File file = new File(MyContextListener.getContextPath() + filePath);
        if (!file.exists()) {
            getHttpResponse().sendError(HttpServletResponse.SC_NOT_FOUND);
            return null;
        }

        getHttpResponse().setContentType("application/force-download");
        getHttpResponse().setContentLength((int) file.length());
        getHttpResponse().setHeader("Content-Disposition", "attachment; filename=" + filePath.split("/")[1]);

        FileUtils.copyFile(file, getHttpResponse().getOutputStream());
        log.info("file downloaded: " + filePath);

        return null;
    }

    public void setFilePath(String filePath) {
        this.filePath = filePath;
    }

}
