/**
 * Copyright (C) 2013 Shenshen Han
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 *
 */
package iu.slam.database;

import iu.slam.models.Experiment;
import iu.slam.models.Photo;
import iu.slam.models.PhotoDrawTag;
import iu.slam.models.PhotoTags;
import iu.slam.models.SensorData;
import iu.slam.models.Video;
import iu.slam.utils.UtilLog;
import java.util.List;
import java.util.Map;
import org.hibernate.Query;
import org.hibernate.Session;
import org.hibernate.SessionFactory;
import org.hibernate.cfg.Configuration;
import org.hibernate.internal.SessionFactoryImpl;
import org.hibernate.service.ServiceRegistryBuilder;
import org.hibernate.service.jdbc.connections.internal.C3P0ConnectionProvider;
import org.hibernate.service.jdbc.connections.spi.ConnectionProvider;
import org.hibernate.transform.AliasToEntityMapResultTransformer;

public class DatabaseHandler {

    private static DatabaseHandler instance;

    public static DatabaseHandler getInstance() {
        if (instance == null) {
            instance = new DatabaseHandler();
        }
        return instance;
    }

    public static List<Map<String, Object>> Query(String sql, Session s) {
        Query q = s.createSQLQuery(sql).setResultTransformer(AliasToEntityMapResultTransformer.INSTANCE);
        @SuppressWarnings("unchecked")
        List<Map<String, Object>> r = q.list();
        return r;
    }

    public static Session getSession() {
        return DatabaseHandler.getInstance().getHibernateSession();
    }

    public static void closeSession(Session session) {
        String s = "SLAM Close Hibernate Session: " + session.hashCode();
        session.close();
        //log.info(s);
    }
    private final SessionFactory sessionFactory;

    private DatabaseHandler() {
        Configuration configuration = new Configuration();
        configuration.setProperty("hibernate.dialect", "org.hibernate.dialect.MySQLDialect");
        configuration.setProperty("hibernate.connection.driver_class", "com.mysql.jdbc.Driver");
        configuration.setProperty("hibernate.connection.url", "jdbc:mysql://127.0.0.1:3306/slam?autoReconnect=true");//3306 wamp mysql
        configuration.setProperty("hibernate.connection.username", "root");
        configuration.setProperty("hibernate.connection.password", "");
        //configuration.setProperty("hibernate.format_sql", "true");
        //configuration.setProperty("hibernate.show_sql", "true");
        configuration.setProperty("hibernate.connection.provider_class", "org.hibernate.service.jdbc.connections.internal.C3P0ConnectionProvider");
        configuration.setProperty("hibernate.c3p0.idleConnectionTestPeriod", "600");
        configuration.setProperty("hibernate.c3p0.testConnectionOnCheckin", "true");

        configuration.addAnnotatedClass(SensorData.class)
                .addAnnotatedClass(Experiment.class)
                .addAnnotatedClass(Photo.class)
                .addAnnotatedClass(Video.class)
                .addAnnotatedClass(PhotoTags.class)
                .addAnnotatedClass(PhotoDrawTag.class);

        sessionFactory = configuration.buildSessionFactory(new ServiceRegistryBuilder().applySettings(configuration.getProperties()).buildServiceRegistry());
        sessionFactory.getStatistics().setStatisticsEnabled(true);

        UtilLog.logDatabase(DatabaseHandler.class, "SessionFactory created");
    }

    public synchronized void clean() {
        if (sessionFactory instanceof SessionFactoryImpl) {
            SessionFactoryImpl sf = (SessionFactoryImpl) sessionFactory;
            ConnectionProvider conn = sf.getConnectionProvider();
            if (conn instanceof C3P0ConnectionProvider) {
                ((C3P0ConnectionProvider) conn).close();
                UtilLog.logDatabase(this, "C3P0ConnectionProvider closed");
            }
        }
        sessionFactory.close();

        instance = null;
        UtilLog.logDatabase(this, "clean DatabaseHandler " + sessionFactory.isClosed());
    }

    private synchronized Session getHibernateSession() {
        Session session = sessionFactory.openSession();
        String s = "SLAM Distribute new Hibernate Session: " + session.hashCode();
        //log.info(s);
        return session;
    }
}
