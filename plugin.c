#include <libyang/libyang.h>
#include <sysrepo.h>
#include <sysrepo/xpath.h>

LY_ERR mountpoint_ext_data_clb(
    const struct lysc_ext_instance *ext,
    void *user_data,
    void **ext_data,
    ly_bool *ext_data_free);

int main(){
    sr_conn_ctx_t *conn;
    sr_session_ctx_t *session;
    sr_subscription_ctx_t *subscription;

    //Set everything up
    int err = sr_connect(SR_CONN_DEFAULT, &conn);
    if (err != SR_ERR_OK){
        printf("connection error %d\n", err);
        return 1;
    }
    
    err = sr_session_start(conn, SR_DS_OPERATIONAL, &session);
    if (err != SR_ERR_OK) {
        printf("session error %d\n", err);
        return 1;
    }

    struct ly_ctx *ctx = sr_acquire_context(conn);
    if (ctx == NULL) {
        return 1;
    }

    sr_set_ext_data_cb(conn, mountpoint_ext_data_clb, (void*) ctx);

    //Create a device, and a hardware component inside its mountpoint
    struct lyd_node** hardwareParent;
    
    LY_ERR lyErr = lyd_new_path(NULL, ctx, "/bbf-device-aggregation:devices/device[name='test']/type", "bbf-device-types:onu", 0, hardwareParent);
    if (lyErr != LY_SUCCESS) {
        printf("device type node creation error %d\n", lyErr);
        return 1;
    }

    printf("device node created\n");

    lyErr = lyd_new_path(*hardwareParent, ctx, "/bbf-device-aggregation:devices/device[name='device-example']/data/ietf-hardware:hardware/component[name='test']/class", "iana-hardware:unknown", 0, NULL);
    
    if (lyErr != LY_SUCCESS) {
        printf("hardware component class creation error %d\n", lyErr);
        return 1;
    }

    printf("hardware component node created\n");

    //Edit the datastore by adding this component
    err = sr_edit_batch(session, *hardwareParent, "merge");
    if (err != SR_ERR_OK) {
        printf("edit batch error %d\n", err);
        return 1;
    }

    printf("edit batch\n");

    err = sr_apply_changes(session, 0);
    if (err != SR_ERR_OK) {
        printf("apply changes error %d\n", err);
        return 1;
    }

    lyd_free_all(*hardwareParent);

    printf("apply changes\n");

    //Try to send a notification with a leafref to ietf-hardware
    struct lyd_node** notificationParent;

    lyErr = lyd_new_path(NULL, ctx, "/bbf-device-aggregation:devices/device[name='device-example']/data/ietf-hardware:hardware-state-oper-enabled/name", "test", 0, notificationParent);
    if (lyErr != LY_SUCCESS) {
        printf("notification creation error %d\n", lyErr);
        return 1;
    }

    err = sr_notif_send_tree(session, *notificationParent, 0, 0);
    if (err != SR_ERR_OK) {
        printf("send notification error %d\n", err);
        return 1;
    }

    printf("notification sent\n");

    lyd_free_all(*notificationParent);

    sr_release_context(conn);

    err = sr_session_stop(session);
    if (err != SR_ERR_OK) {
        printf("cleanup session error %d\n", err);
        return 1;
    }
    err = sr_disconnect(conn);
    if (err != SR_ERR_OK) {
        printf("cleanup connection error %d\n", err);
        return 1;
    }
    return 0;
}

LY_ERR mountpoint_ext_data_clb(
    const struct lysc_ext_instance *ext,
    void *user_data,
    void **ext_data,
    ly_bool *ext_data_free)
{
    struct ly_ctx *ctx = (struct ly_ctx*) user_data;

    LY_ERR lyErr = lyd_parse_data_path(ctx, "/conf/schema-mount.xml", LYD_XML, LYD_PARSE_STRICT, LYD_VALIDATE_PRESENT, ext_data);
    if (lyErr != LY_SUCCESS) {
        printf("schema mount data parsing failed %d\n", lyErr);
        return 1;
    }

    *ext_data_free = 1;
    return LY_SUCCESS;
}