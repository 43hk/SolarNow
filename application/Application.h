#pragma once
#include <cstdint>


//防止头文件重复包含，直接将GLFW相关定义为类
class GLFWwindow;


using ResizeCallback = void(*)(int width, int height);
using KeyBoardCallback = void(*)(int key, int action, int mods);

class Application
{
public:
	~Application();

	//用于访问静态实例的函数
	static Application *getInstance();

	bool init(const int &width = 1920 ,const int &height = 1080);

	bool update();

	void destroy();

	//将指针指向传入的函数 4
	void setResizeCallback(ResizeCallback callback)
	{
		mResizeCallback = callback;
	}

	void setKeyBoardCallback(KeyBoardCallback callback)
	{
		mKeyBoardCallBack = callback;
	}


	//用于获取长宽的接口
	uint32_t getWidth() const { return mWidth; }
	uint32_t getHeight() const { return mHeight; }


private:
	static void framebufferSizeCallback(GLFWwindow *window, int width, int height);
	static void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

	//全局唯一静态变量实例
	static Application *mInstance;

	uint32_t mWidth { 0 };
	uint32_t mHeight { 0 };

	GLFWwindow *mWindow { nullptr };

	//创建空指针 3
	ResizeCallback mResizeCallback{ nullptr };
	KeyBoardCallback mKeyBoardCallBack{ nullptr };

	Application();
};

