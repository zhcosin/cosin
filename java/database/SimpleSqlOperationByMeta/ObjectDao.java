package com.cosin.database.simplesql;

import java.lang.reflect.Field;
import java.math.BigDecimal;
import java.sql.Connection;
import java.sql.SQLException;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.sql.DataSource;

import com.alibaba.fastjson.JSONObject;

import org.apache.poi.hssf.record.formula.functions.T;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.beans.factory.annotation.Qualifier;
import org.springframework.stereotype.Component;

@Component
public class ObjectDao {
    private Map<String, ObjectMetaInfo> objMetaInfoMap = new HashMap<>();

    @Autowired
	@Qualifier("myDataSource")
    private DataSource dataSource;
    
    private final String dateFormatStr = "yyyy-MM-dd HH:mm:ss";

	private final SimpleDateFormat dateFormat = new SimpleDateFormat(dateFormatStr);

    private final Logger log = LoggerFactory.getLogger(ObjectDao.class);

    public ObjectDao() {
        initObjectMetaInfos();
    }
    
    public void initObjectMetaInfos() {
        ObjectMetaInfo sampleMetaInfo = new ObjectMetaInfo();
	sampleMetaInfo.setObjectName("Car");
	sampleMetaInfo.setTableName("car");
	sampleMetaInfo.setIdAttrName("id");
        Map<String, ObjectFieldMetaInfo> carFields = new HashMap<>();
        carFields.put("id", new ObjectFieldMetaInfo("id", "id", ObjectFieldType.StringType));
        carFields.put("displacement", new ObjectFieldMetaInfo("displacement", "displacement", ObjectFieldType.FloatType));
        carFields.put("power", new ObjectFieldMetaInfo("power", "power", ObjectFieldType.StringType));
        carFields.put("owner", new ObjectFieldMetaInfo("owner", "owner", ObjectFieldType.StringType));
        sampleMetaInfo.setFieldInfos(robotFields);
        objMetaInfoMap.put(sampleMetaInfo.getObjectName(), sampleMetaInfo);
    }

    private String dateToString(Date time) {
		if (time == null)
			return null;

		return dateFormat.format(time);
	}

    private boolean executeSqlUpdate(String sql) {
        Connection conn = null;
		try {
			conn = impalaDataSource.getConnection();
			conn.createStatement().executeUpdate(sql);
			return true;
		} catch (Exception e) {
			log.error("", e);
			return false;
		} finally {
			if (conn != null) {
				try {
					conn.close();
				} catch (SQLException e) {
					log.error("", e);
				}
			}
		}
    }

    public boolean addObjectInfoByJson(String objectName, Object key, JSONObject dataJson) {
        Map<String, Object> dataMap = JSONObject.toJavaObject(dataJson, Map.class);
        return addObjectInfo(objectName, key, dataMap);
    }

    public boolean updateObjectInfoByJson(String objectName, Object key, JSONObject dataJson) {
        Map<String, Object> dataMap = JSONObject.toJavaObject(dataJson, Map.class);
        return updateObjectInfo(objectName, key, dataMap);
    }

    private boolean addObjectInfo(String objectName, Object key, Map<String, Object> data) {
        ObjectMetaInfo objMetaInfo = objMetaInfoMap.get(objectName);
        if (objMetaInfo == null) {
            log.error(String.format("The meta info of object %s not found.", objectName));
            return false;
        }

        data.put(objMetaInfo.getIdAttrName(), key);

        String sql = buildSqlForInsert(objMetaInfo, data);
        return executeSqlUpdate(sql);
    }

    private boolean updateObjectInfo(String objectName, Object key, Map<String, Object> data) {
        ObjectMetaInfo objMetaInfo = objMetaInfoMap.get(objectName);
        if (objMetaInfo == null) {
            log.error(String.format("The meta info of object %s not found.", objectName));
            return false;
        }

        data.put(objMetaInfo.getIdAttrName(), key);

        String sql = buildSqlForUpdate(objMetaInfo, data);
        return executeSqlUpdate(sql);
    }

    public boolean deleteObjectInfo(String objectName, Object key) {
        ObjectMetaInfo objMetaInfo = objMetaInfoMap.get(objectName);
        if (objMetaInfo == null) {
            log.error(String.format("The meta info of object %s not found.", objectName));
            return false;
        }

        String sql = buildSqlForDelete(objMetaInfo, key);
        return executeSqlUpdate(sql);
    }

    private String buildSqlForInsert(ObjectMetaInfo objMetaInfo, Map<String, Object> data) {
        List<String> fieldList = new ArrayList<>();
        List<String> valueList = new ArrayList<>(); 

        for (String attrName : objMetaInfo.getFieldInfos().keySet()) {
            ObjectFieldMetaInfo fieldMetaInfo = objMetaInfo.getFieldInfos().get(attrName);
            Object valueWaitForWrite = data.get(attrName);

            fieldList.add(fieldMetaInfo.getTableFieldname());
            valueList.add(buildSqlValueStr(fieldMetaInfo, valueWaitForWrite));
        }

        String sql = String.format("insert into %s(%s) values(%s)", 
            objMetaInfo.getTableName(), 
            strListToStr(fieldList, ","),
            strListToStr(valueList, ","));
        
        return sql;
    }

    private String buildSqlForUpdate(ObjectMetaInfo objMetaInfo, Map<String, Object> data) {
        List<String> fieldUpdateSetList = new ArrayList<>();

        // 生成字段更新 set 子句
        for (String attrName : objMetaInfo.getFieldInfos().keySet()) {
            if (attrName.equals(objMetaInfo.getIdAttrName())) {
                continue; // id 不能更新
            }

            ObjectFieldMetaInfo fieldMetaInfo = objMetaInfo.getFieldInfos().get(attrName);
            Object valueWaitForWrite = data.get(attrName);

            fieldUpdateSetList.add(String.format("%s=%s", 
                fieldMetaInfo.getTableFieldname(), 
                buildSqlValueStr(fieldMetaInfo, valueWaitForWrite)));
        }

        // 生成 where 条件
        ObjectFieldMetaInfo idfieldMetaInfo = objMetaInfo.getFieldInfos().get(objMetaInfo.getIdAttrName());
        String whereClause = String.format("%s=%s", idfieldMetaInfo.getTableFieldname(), buildSqlValueStr(idfieldMetaInfo, data.get(objMetaInfo.getIdAttrName()))); 

        String sql = String.format("update %s set %s where %s", 
            objMetaInfo.getTableName(), 
            strListToStr(fieldUpdateSetList, ","),
            whereClause);
        
        return sql;
    }

    private String buildSqlForDelete(ObjectMetaInfo objMetaInfo, Object key) {
         // 生成 where 条件
         ObjectFieldMetaInfo idfieldMetaInfo = objMetaInfo.getFieldInfos().get(objMetaInfo.getIdAttrName());
         String whereClause = String.format("%s=%s", idfieldMetaInfo.getTableFieldname(), buildSqlValueStr(idfieldMetaInfo, key)); 
 
         String sql = String.format("delete from %s where %s", 
             objMetaInfo.getTableName(), 
             whereClause);
         
         return sql;
    }

    private String buildSqlValueStr( ObjectFieldMetaInfo fieldMetaInfo, Object value) {
        if (value == null) {
            return "null";
        } else {
            switch (fieldMetaInfo.getType()) {
                case StringType: {
                    if (fieldMetaInfo.isMayContainChinese()) {
                        return String.format("cast('%s' as string)", value);
                    } else {
                        return String.format("'%s'", value);
                    }
                } case IntegerType: {
                    return String.format("%d", value);
                } case FloatType: {
                    if (value instanceof Integer) {
                       return String.format("%d", value);
                    } else if (value instanceof BigDecimal) {
                        BigDecimal valueInBigDecimal = (BigDecimal)value;
                        return String.format("%f", valueInBigDecimal.floatValue());
                    } else {
                        return String.format("%f", value);
                    }
                } case DoubleType: {
                    if (value instanceof Integer) {
                        return String.format("%d", value);
                     } else if (value instanceof BigDecimal) {
                         BigDecimal valueInBigDecimal = (BigDecimal)value;
                         return String.format("%f", valueInBigDecimal.doubleValue());
                     } else {
                         return String.format("%f", value);
                     }
                } case DateTimeType: {
                    if (value instanceof Date) {
                        return String.format("%s", dateFormat.format(value));
                    } else if (value instanceof String) {
                        try {
                            return String.format("to_timestamp('%s', '%s')", value, dateFormatStr);
                        } catch (Exception e) {
                            log.error("", e);
                            return "null";
                        }
                    } else {
                        return "null";
                    }
                } default: {
                    return "null";
                }
            }
        }
    }

    private String strListToStr(List<String> strList, String seprator) {
        if (strList == null || seprator == null)
            return null;

        String result = "";
        int len = strList.size();
        for (int i = 0; i < len; ++i) {
            if (i > 0) {
                result += seprator;
            }

            result += strList.get(i);
        }

        return result;
    }
}
