package com.cosin.database.simplesql;

import java.util.Map;

public class ObjectMetaInfo {
    private String objectName;
    private String tableName;
    private String idAttrName;

    private Map<String, ObjectFieldMetaInfo> fieldInfos;

    public String getObjectName() {
        return objectName;
    }

    public void setObjectName(String objectName) {
        this.objectName = objectName;
    }

    public String getTableName() {
        return tableName;
    }

    public void setTableName(String tableName) {
        this.tableName = tableName;
    }

    public String getIdAttrName() {
        return idAttrName;
    }

    public void setIdAttrName(String idAttrName) {
        this.idAttrName = idAttrName;
    }

    public Map<String, ObjectFieldMetaInfo> getFieldInfos() {
        return fieldInfos;
    }

    public void setFieldInfos(Map<String, ObjectFieldMetaInfo> fieldInfos) {
        this.fieldInfos = fieldInfos;
    }
}
