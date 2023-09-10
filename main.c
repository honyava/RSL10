#include <RSL10.h>

void Initialize(void)
{



}

int main(void)
{

    Initialize();
    int t = 0;
    while (1)
    {
      t++;
      if (t == 10) t = 0;
    }
}