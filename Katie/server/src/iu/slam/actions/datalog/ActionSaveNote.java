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
public class ActionSaveNote extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private static final Log log = LogFactory.getLog(ActionSaveNote.class);
    private String experimentId;
    private String note;

    @Action("/saveNote")
    @Override
    public String execute() throws Exception {
        Session s = DatabaseHandler.getSession();
        try {
            Experiment.updateNote(experimentId, note, s);
            log.info("note updated for: " + experimentId);
        } catch (Exception e) {
            log.warn(e);
        } finally {
            DatabaseHandler.closeSession(s);
        }
        return null;
    }

    public void setExperimentId(String experimentId) {
        this.experimentId = experimentId;
    }

    public void setNote(String note) {
        this.note = note;
    }

}
