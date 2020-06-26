sprint(
    "JSON Version from LUA " .. json.parse(file_string("/versions")).os.version .. " "
)

save_string("main", "APP", "scal")