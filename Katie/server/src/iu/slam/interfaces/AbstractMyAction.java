package iu.slam.interfaces;

import com.opensymphony.xwork2.ActionSupport;
import java.util.Map;
import javax.servlet.ServletContext;
import javax.servlet.http.HttpServletResponse;
import org.apache.struts2.interceptor.ServletResponseAware;
import org.apache.struts2.interceptor.SessionAware;
import org.apache.struts2.util.ServletContextAware;

/**
 *
 * @author hanaldo
 */
public abstract class AbstractMyAction extends ActionSupport implements SessionAware, ServletResponseAware, ServletContextAware {

    public static final String TOP = "top";
    private static final long serialVersionUID = 1L;
    private HttpServletResponse httpResponse;
    private Map<String, Object> httpSession;
    private ServletContext servletContext;

    @Override
    public abstract String execute() throws Exception;

    public HttpServletResponse getHttpResponse() {
        return httpResponse;
    }

    public Map<String, Object> getHttpSession() {
        return httpSession;
    }

    public ServletContext getServletContext() {
        return servletContext;
    }

    @Override
    public void setServletResponse(HttpServletResponse hsr) {
        httpResponse = hsr;
    }

    @Override
    public void setSession(Map<String, Object> map) {
        httpSession = map;
    }

    @Override
    public void setServletContext(ServletContext sc) {
        servletContext = sc;
    }
}
