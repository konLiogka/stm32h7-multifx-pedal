
#include "editPedal.hpp" 
#include <cstring>

#define PEDAL_PAGE_START 11
#define PEDAL_SEC_WORD_PAGE_START 12
#define PEDAL_COLUMN_START 50
#define PEDAL_NAME_MAX_LENGTH 10 


void displaySelectedPedal(Pedal *pedal)
{
    const Bitmap& bmp = pedal->getImage();
    const char* name  = pedal->getName();
    Display::clear();

    Display::drawBitmap(edit_pedal_bitmap, 0, 0);
    Display::drawBitmap(bmp, 20, PEDAL_PAGE_START);

    if (strlen(name) > PEDAL_NAME_MAX_LENGTH) {

        const char* space = strchr(name, ' ');

        if (space && *(space + 1)) {

            size_t len = std::min<size_t>(space - name, PEDAL_NAME_MAX_LENGTH);
            char firstWord[PEDAL_NAME_MAX_LENGTH + 1] = {0};
            strncpy(firstWord, name, len);
            firstWord[len] = '\0';

            Display::drawString(firstWord, PEDAL_COLUMN_START, PEDAL_PAGE_START);
            Display::drawString(space + 1, PEDAL_COLUMN_START, PEDAL_SEC_WORD_PAGE_START);
            return;
        }
    }
    Display::drawString(name, PEDAL_COLUMN_START, PEDAL_PAGE_START);
}
