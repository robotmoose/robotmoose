package iu.slam.actions.datalog;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.Experiment;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Session;

/**
 *
 * @author hanaldo
 */
public class ActionGetNote extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private static final Log log = LogFactory.getLog(ActionGetNote.class);
    private String experimentId;

    @Action("/getNote")
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        String note = null;
        try {
            note = Experiment.getNote(experimentId, s);
        } catch (Exception e) {
            log.warn(e);
        } finally {
            DatabaseHandler.closeSession(s);
        }

        if (note == null) {
            getHttpResponse().getWriter().println("");
        } else {
            getHttpResponse().getWriter().println(note);
        }

        return null;
    }

    public void setExperimentId(String experimentId) {
        this.experimentId = experimentId;
    }

}
