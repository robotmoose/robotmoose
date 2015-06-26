package iu.slam.protocol;

import iu.slam.utils.MyGson;

/**
 *
 * @author hanaldo
 */
public class ControlMessage {

    public static final String Topic_DataLog = "topic.datalog";
    public static final String Command_DataLog_Start = "command.datalog.start";
    public static final String Command_DataLog_End = "command.datalog.end";
    public static final String Command_DataLog_Check = "command.datalog.check";
    public static final String Topic_Dialog_Count = "topic.dialog.count";
    public static final String Topic_New_Connection = "topic.new";
    public static final String Topic_Refresh = "topic.refresh";
    public static final String Topic_New_Photo = "topic.new.photo";
    public static final String Command_Reset = "command.reset.katie";
    public static final String Command_Take_Photo = "command.take.photo";
    public static final String Command_Report_Status = "command.report.status";

    public static ControlMessage getMessage(String json) {
        return MyGson.getGSON().fromJson(json, ControlMessage.class);
    }

    private String topic;
    private String command;
    private Object content;

    public ControlMessage(String topic, String command) {
        this.topic = topic;
        this.command = command;
    }

    public String getTopic() {
        return topic;
    }

    public void setTopic(String topic) {
        this.topic = topic;
    }

    public Object getCommand() {
        return command;
    }

    public void setCommand(String command) {
        this.command = command;
    }

    public Object getContent() {
        return content;
    }

    public void setContent(Object content) {
        this.content = content;
    }

    @Override
    public String toString() {
        return MyGson.getGSON().toJson(this);
    }
}
