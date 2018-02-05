#include <X11/Xlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

static int error_handler(Display *display, XErrorEvent *error)
{
    return 0;
}

int rp_select_window(Window window)
{
    int link[2];
    pid_t pid;

    pipe(link);

    pid = fork();

    if(pid == 0)
    {
        dup2(link[1], STDOUT_FILENO);
        close(link[0]);
        close(link[1]);
        execlp("ratpoison", "ratpoison", "-c", "windows %i:%f:%n", (char *) NULL);
        _exit(EXIT_FAILURE);
    }

    char buf[4096];

    close(link[1]);
    read(link[0], buf, sizeof(buf));
    wait(NULL);

    char *token;

    token = strtok(buf, ":\n");

    while(token != NULL)
    {
        if(atol(token) == (long) window)
        {
            char fselect[32];
            strcpy(fselect, "fselect ");
            token = strtok(NULL, ":\n");
            if(strcmp(token, " ") == 0)
            {
                strcat(fselect, "-1");
            }
            else
            {
                strcat(fselect, token);
            }

            char select[32];
            strcpy(select, "select ");
            strcat(select, strtok(NULL, ":\n"));

            pid = fork();
            if(pid == 0)
            {
                execlp("ratpoison", "ratpoison", "-i", "-c", fselect, "-c", select, "-c", "curframe", (char *) NULL);
                _exit(EXIT_FAILURE);
            }

            wait(NULL);

            break;
        }
        else
        {
            strtok(NULL, ":\n");
            strtok(NULL, ":\n");
            token = strtok(NULL, ":\n");
        }
    }

    return token == NULL;
}

int main()
{
  Display *display;

  display = XOpenDisplay(NULL);
  if(!display)
  {
      return 1;
  }

  XSetErrorHandler(error_handler);

  for(int i = 0; i < ScreenCount(display); i++)
  {
      Window root;
      Window parent;
      Window *children;
      unsigned int num_children;

      XSelectInput(display, RootWindow(display, i), SubstructureNotifyMask);
      XQueryTree(display, RootWindow(display, i), &root, &parent, &children, &num_children);
      for(unsigned int j = 0; j < num_children; j++)
      {
          XGrabButton(display, Button1, AnyModifier, children[j], False, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
      }
  }

  XEvent event;
  while(1)
  {
      XNextEvent(display, &event);
      switch(event.type)
      {
          case CreateNotify:
              XGrabButton(display, Button1, AnyModifier, event.xcreatewindow.window, False, ButtonPressMask, GrabModeSync, GrabModeSync, None, None);
              break;
          case ButtonPress:
              if(rp_select_window(event.xbutton.window))
              {
                  XSetInputFocus(display, event.xbutton.window, RevertToParent, CurrentTime);
              }
              XAllowEvents(display, ReplayPointer, CurrentTime);
              break;
      }
  }

  XCloseDisplay(display);
  return 0;
}
