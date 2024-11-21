#include "Application.h"

#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>



Application *Application::mInstance = nullptr;
Application *Application::getInstance()
{

    if (mInstance == nullptr)
    {
        mInstance = new Application();
    }

    return mInstance;
}


Application::~Application() = default;
Application::Application() = default;


bool Application::init(const int &width, const int &height)
{

    mWidth = width;
    mHeight = height;

    //初始化glfw
    glfwInit();
    //设置版本号
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    //设置核心模式
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


    //指向创建的窗体
    mWindow = glfwCreateWindow(mWidth, mHeight, "SolarNow", nullptr, nullptr);

    if (mWindow == nullptr)
    {
        return false;
    }

    //传入指针到上下文
    glfwMakeContextCurrent(mWindow);

    //加载GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "failed to load GLAD" << std::endl;
        return false;
    }

    //窗口大小调整的回调函数 1
    glfwSetFramebufferSizeCallback(mWindow, framebufferSizeCallback);
    //glfw键盘响应 1
    glfwSetKeyCallback(mWindow, keyCallback);

    //this指向全局唯一Application对象,将该指针传入窗体对象
    glfwSetWindowUserPointer(mWindow, this);

    return true;
}


bool Application::update()
{
    if (glfwWindowShouldClose(mWindow))
    {
        return false;
    }

    //接受并分发窗口消息（处理本次循环的消息）
    glfwPollEvents();

    //切换双缓存
    glfwSwapBuffers(mWindow);

    return true;
}


void Application::destroy()
{

    glfwTerminate();
}

//通过指针传入数据 2
void Application::framebufferSizeCallback(GLFWwindow *window, int width, int height)
{
    //this是空类型指针，需要强制转换类型
    Application *self = (Application *)glfwGetWindowUserPointer(window);

    if (self->mResizeCallback != nullptr)
    {
        self->mResizeCallback(width, height);
    }

    //等效于
    /* if (Application::getInstance()->mResizeCallback != nullptr)
    {
        Application::getInstance()->mResizeCallback(width, height);
    }*/
}


void Application::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    Application *self = (Application *)glfwGetWindowUserPointer(window);
    
    if (self->mKeyBoardCallBack != nullptr)
    {
        self->mKeyBoardCallBack(key, action, mods);
    }
}

