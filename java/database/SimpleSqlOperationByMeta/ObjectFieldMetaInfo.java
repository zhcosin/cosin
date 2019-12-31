package com.cosin.database.simplesql;

public class ObjectFieldMetaInfo {
    private String name;
    private String tableFieldname;
    private ObjectFieldType type;
    private boolean nullable = true;
    private Object defaultValue = null;
    private boolean mayContainChinese = false;

    public ObjectFieldMetaInfo(String name, String tableFieldname, ObjectFieldType type) {
        this.name = name;
        this.tableFieldname = tableFieldname;
        this.type = type;
    }

    public ObjectFieldMetaInfo(String name, String tableFieldname, ObjectFieldType type, Object defaultValue) {
        this.name = name;
        this.tableFieldname = tableFieldname;
        this.type = type;
        this.defaultValue = defaultValue;
    }

    public ObjectFieldMetaInfo(String name, String tableFieldname, ObjectFieldType type, boolean nullable, Object defaultValue,
            boolean mayContainChinese) {
        this.name = name;
        this.tableFieldname = tableFieldname;
        this.type = type;
        this.nullable = nullable;
        this.defaultValue = defaultValue;
        this.mayContainChinese = mayContainChinese;
    }

    public String getName() {
        return name;
    }

    public void setName(String name) {
        this.name = name;
    }

    public ObjectFieldType getType() {
        return type;
    }

    public void setType(ObjectFieldType type) {
        this.type = type;
    }

    public boolean isNullable() {
        return nullable;
    }

    public void setNullable(boolean nullable) {
        this.nullable = nullable;
    }

    public Object getDefaultValue() {
        return defaultValue;
    }

    public void setDefaultValue(Object defaultValue) {
        this.defaultValue = defaultValue;
    }

    public boolean isMayContainChinese() {
        return mayContainChinese;
    }

    public void setMayContainChinese(boolean mayContainChinese) {
        this.mayContainChinese = mayContainChinese;
    }

    public String getTableFieldname() {
        return tableFieldname;
    }

    public void setTableFieldname(String tableFieldname) {
        this.tableFieldname = tableFieldname;
    }
}
