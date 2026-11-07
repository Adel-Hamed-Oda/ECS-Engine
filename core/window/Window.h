#pragma once

#include "core/import_files/private_imports.h"

class Window
{
public:
    inline static Window* GetPrimaryWindow() { return AllWindows[0]; }

    Window(
        const char* title = "New window",
        glm::ivec2 position = glm::ivec2(0, 0),
        glm::ivec2 size = glm::ivec2(1920, 1080)
    ) {
        id = nextId++;

        GLFWmonitor* monitor = glfwGetPrimaryMonitor();
        m_window = glfwCreateWindow(size.x, size.y, title, monitor, nullptr);
        glfwSetWindowPos(m_window, position.x, position.y);

        AllWindows.push_back(this);
    }

	~Window() {
        AllWindows.erase(std::remove(AllWindows.begin(), AllWindows.end(), this), AllWindows.end());
        if (AllWindows.size() > 0) glfwMakeContextCurrent(AllWindows[0]->m_window);
        glfwDestroyWindow(m_window);
    }
	
    int GetId() const { return id; }
    glm::vec2 GetPosition() const {
		int x, y;
		glfwGetWindowPos(m_window, &x, &y);
		return glm::vec2(x, y);
	}
	glm::vec2 GetSize() const
	{
		int width, height;
		glfwGetWindowSize(m_window, &width, &height);
		return glm::vec2(width, height);
	}
	std::string GetTitle() const
	{
		return std::string(glfwGetWindowTitle(m_window));
	}

	void SetPosition(int x, int y)
	{
		glfwSetWindowPos(m_window, x, y);
	}
	void SetSize(float width = -1.0f, float height = -1.0f)
	{
		glm::vec2 size = GetSize();
		if (width == -1.0f) width = size.x;
		if (height == -1.0f) height = size.y;
		glfwSetWindowSize(m_window, (int)width, (int)height);
	}
	void SetTitle(const std::string& title) { glfwSetWindowTitle(m_window, title.c_str()); }
	void SetAsPrimaryWindow() { 
        if (AllWindows.size() > 0) {
            std::swap(AllWindows[0], AllWindows[id]);
        }
        glfwMakeContextCurrent(m_window);
    }

    static Window* Get(int id) { return AllWindows[id]; }

private:
    inline static std::vector<Window*> AllWindows = std::vector<Window*>();
    inline static int nextId = 0;
    int id;
	
    GLFWwindow* m_window;
};
