package iu.slam.actions.datalog;

import iu.slam.database.DatabaseHandler;
import iu.slam.interfaces.AbstractMyAction;
import iu.slam.models.SensorData;
import iu.slam.utils.MyContextListener;
import iu.slam.utils.MyGson;
import iu.slam.utils.UtilLog;
import java.io.File;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.LinkedList;
import java.util.List;
import java.util.Scanner;
import org.apache.http.HttpStatus;
import org.apache.struts2.convention.annotation.Action;
import org.hibernate.Criteria;
import org.hibernate.Session;
import org.hibernate.criterion.Restrictions;

/**
 *
 * @author hanaldo
 */
public class ActionGetValues extends AbstractMyAction {

    private static final long serialVersionUID = 1L;
    private int robotId = -1;
    private long startTime = -1;

    @Action("/getDataValues")
    @Override
    public String execute() throws Exception {
        if (robotId < 0 || startTime < 0) {
            UtilLog.logWarn(this, "RobotId or Day is not set");
            getHttpResponse().sendError(HttpStatus.SC_BAD_REQUEST, "RobotId or Day is not set");
            return null;
        }

        Date start = new Date(startTime);
        Date end = new Date(startTime + 1000 * 60 * 5);
        UtilLog.logInfo(this, "getDataValues for robot " + robotId + " and for " + start);

        Session s = DatabaseHandler.getSession();
        try {
            Criteria c = s.createCriteria(SensorData.class);
            c.add(Restrictions.eq("robotId", robotId));
            c.add(Restrictions.ge("startTime", start));
            c.add(Restrictions.lt("startTime", end));
            @SuppressWarnings("unchecked")
            List<SensorData> l = c.list();
            List<Double> ySeries = new LinkedList<>();
            List<Double> ySeries2 = new LinkedList<>();
            List<String> categories = new LinkedList<>();

            SimpleDateFormat format = new SimpleDateFormat("HH:mm:ss");

            for (SensorData record : l) {
                File file = new File(MyContextListener.getContextPath() + record.getFilePath());
                if (!file.exists()) {
                    continue;
                }
                try (Scanner scan = new Scanner(file)) {
                    while (scan.hasNextLine()) {
                        String[] tokens = scan.nextLine().split("\\s+");
                        if (tokens.length < 13) {
                            continue;
                        }
                        ySeries.add(Double.parseDouble(tokens[4]));
                        ySeries2.add(Double.parseDouble(tokens[5]));
                        categories.add(format.format(new Date(Long.parseLong(tokens[12]))));
                    }
                }
            }

            YDataPackage series = new YDataPackage("IR Value 1", ySeries);
            YDataPackage series2 = new YDataPackage("IR Value 2", ySeries2);
            ResultPackage result = new ResultPackage(categories, new YDataPackage[]{series, series2});
            getHttpResponse().getWriter().println(MyGson.getGSON().toJson(result));
        } finally {
            DatabaseHandler.closeSession(s);
        }
        return null;
    }

    public void setRobotId(int robotId) {
        this.robotId = robotId;
    }

    public void setStartTime(long startTime) {
        this.startTime = startTime;
    }

    private class ResultPackage {

        private List<String> categories;
        private YDataPackage[] series;

        public ResultPackage(List<String> categories, YDataPackage[] series) {
            this.categories = categories;
            this.series = series;
        }
    }

    private class YDataPackage {

        private String name;
        private List<Double> data;

        public YDataPackage(String name, List<Double> data) {
            this.name = name;
            this.data = data;
        }

    }
}
