/* flags: --c-std=gnu17 */
/* A GNU dialect's #import: the file is included, and not again (by #import
   or #include). */
#import "inc/imported.h"
#import "inc/imported.h"
#include "inc/imported.h"
end
