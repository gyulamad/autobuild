#include "../cpptools/misc/tpl_replace.hpp"
#include "../cpptools/misc/get_cwd.hpp"
#include "../cpptools/misc/file_get_contents.hpp"
#include "../cpptools/misc/file_put_contents.hpp"

int main() {
    return !file_put_contents(".clangd", 
        tpl_replace({
            { "{{PROJECT_ROOT}}", get_cwd() }
        }, file_get_contents(".clangd.tpl"))
    );
}