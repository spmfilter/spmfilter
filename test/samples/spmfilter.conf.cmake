[global]
engine = smtpd
lib_dir = ${CMAKE_CURRENT_BINARY_DIR}/test
foreground = true
add_header = false
queue_dir = ${CMAKE_CURRENT_BINARY_DIR}/test
bind_ip = ${TEST_BINDIP}
bind_port = ${TEST_BINDPORT}
nexthop = ${TEST_NEXTHOP}
spare_childs = 2
max_childs = 10
pid_file = ${CMAKE_CURRENT_BINARY_DIR}/test/smf_test_smtpd.pid
modules = testmod1, testmod2
module_fail = 2
debug = false