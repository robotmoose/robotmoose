package iu.slam.actions.datalog;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.SensorData;
import iu.slam.utils.MyContextListener;
import iu.slam.utils.UtilLog;
import java.io.File;
import java.io.IOException;
import java.util.LinkedList;
import java.util.List;
import javax.servlet.http.HttpServletResponse;
import org.apache.commons.io.FileUtils;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
public class ActionGetExperimentLog extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private String experimentId;

    @Action("/getExpFile")
    @Override
    public String execute() throws Exception {
        LinkedList<File> files = new LinkedList<>();
        int totalSize = 0;
        Session s = DatabaseHandler.getSession();
        try {
            List<SensorData> records = SensorData.listExperimentData(experimentId, s);
            for (SensorData record : records) {
                File file = new File(MyContextListener.getContextPath() + record.getFilePath());
                if (!file.exists()) {
                    getHttpResponse().sendError(HttpServletResponse.SC_NOT_FOUND);
                    return null;
                }
                totalSize += (int) file.length();
                files.add(file);
            }
        } catch (IOException e) {
            UtilLog.logWarn(this, e);
        } finally {
            DatabaseHandler.closeSession(s);
        }

        getHttpResponse().setContentType("application/force-download");
        getHttpResponse().setContentLength(totalSize);
        getHttpResponse().setHeader("Content-Disposition", "attachment; filename=exp-" + experimentId + ".txt");
        for (File file : files) {
            if (!file.exists()) {
                continue;
            }
            FileUtils.copyFile(file, getHttpResponse().getOutputStream());
            UtilLog.logInfo(this, "file [" + file.getName() + "] sent for [" + experimentId + "]");
        }

        return null;
    }

    public void setExperimentId(String experimentId) {
        this.experimentId = experimentId;
    }

}
