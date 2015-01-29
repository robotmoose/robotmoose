package iu.slam.utils;



import java.io.InputStream;
import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.apache.http.Header;
import org.apache.http.HttpResponse;
import org.apache.http.client.methods.HttpGet;
import org.apache.http.impl.client.DefaultHttpClient;

public class HttpUtil {

    private static final Log log = LogFactory.getLog(HttpUtil.class);

    public static void httpGetData(String uri, Header[] headers) throws Exception {
        log.info("requesting: " + uri);
        DefaultHttpClient httpclient = new DefaultHttpClient();
        httpclient.getParams().setParameter("http.socket.timeout", 10000);
        httpclient.getParams().setParameter("http.connection.timeout", 10000);

        HttpGet get = new HttpGet(uri);
        if (headers != null) {
            for (Header h : headers) {
                get.addHeader(h);
            }
        }
        HttpResponse response = httpclient.execute(get);
        log.info("http: " + response.getStatusLine().getStatusCode());
    }

    public static InputStream httpGetStream(String uri, Header[] headers) throws Exception {
        log.info("requesting: " + uri);
        DefaultHttpClient httpclient = new DefaultHttpClient();
        httpclient.getParams().setParameter("http.socket.timeout", 10000);
        httpclient.getParams().setParameter("http.connection.timeout", 10000);

        HttpGet get = new HttpGet(uri);
        if (headers != null) {
            for (Header h : headers) {
                get.addHeader(h);
            }
        }
        HttpResponse response = httpclient.execute(get);
        log.info("http: " + response.getStatusLine().getStatusCode());
        return response.getEntity().getContent();
    }

    private HttpUtil() {
    }
}
