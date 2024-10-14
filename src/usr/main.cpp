#include "bcknd/vkapp.h"
#include "bcknd/params.h"
#include "engine.h"

int main() {
    Engine eng;
    Vkapp app; 
    GfxParams::inst.msaa  = MSAAvalue::x1;
    app.win.drawArea = {900, 900};
    app.validationEnabled = true;

    app.init();

    eng.run(app);

    app.dstr(); 
    return 0;
}
