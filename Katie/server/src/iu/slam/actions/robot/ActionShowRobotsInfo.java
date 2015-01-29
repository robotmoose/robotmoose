package iu.slam.actions.robot;

import static com.opensymphony.xwork2.Action.SUCCESS;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.RobotInfo;
import java.util.Collection;
import org.apache.struts2.convention.annotation.Action;
import org.apache.struts2.convention.annotation.ParentPackage;
import org.apache.struts2.convention.annotation.Result;

/**
 *
 * @author hanaldo
 */
@ParentPackage("json-default")
public class ActionShowRobotsInfo extends AbstractMyAction {

    private static final long serialVersionUID = 1L;

    @Action(value = "/showRobots", results = {
        @Result(name = SUCCESS, type = "json")
    })
    @Override
    public String execute() throws Exception {
        return SUCCESS;
    }

    public Collection<RobotInfo> getRobots() {
        return RobotConnections.getRobots().values();
    }
}
