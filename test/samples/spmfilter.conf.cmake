[global]
engine = smtpd
lib_dir = ${CMAKE_CURRENT_BINARY_DIR}/test
foreground = true
add_header = false
queue_dir = ${CMAKE_CURRENT_BINARY_DIR}/test
bind_ip = 127.0.0.1
bind_port = 12525
nexthop = ${TEST_NEXTHOP}
spare_childs = 2
max_childs = 10
pid_file = ${CMAKE_CURRENT_BINARY_DIR}/test/smf_test_smtpd.pid
modules = testmod1, testmod2
module_fail = 3
debug = true