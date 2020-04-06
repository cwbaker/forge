---
layout: page
title: Custom Commands
parent: More Details
nav_order: 3
---

Provide custom commands able to be specified on the command line by defining new global functions from *forge.lua* or required modules.

For example the `install()` function below to specifies a new, custom build command by defining a global function.  Passing `install` on the command line will invoke this command to build and install Forge.  The variable `prefix` can be passed on the command line to override the default install location in the user's home directory.

~~~lua
function install()
    prefix = prefix and root( prefix ) or home( 'forge' );
    local failures = build();
    if failures == 0 then 
        cc:cpdir( '${prefix}/bin', '${bin}' );
        cc:cpdir( '${prefix}/lua', 'src/forge/lua' );
    end
end
~~~
