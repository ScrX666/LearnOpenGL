## 配置环境
1. 拉取仓库
2. 下载vcpkg

[https://github.com/microsoft/vcpkg](https://github.com/microsoft/vcpkg)

3. 下载cmake

[https://cmake.org/download/](https://cmake.org/download/)

4. <font style="color:rgb(25, 27, 31);">安装glad库</font>

`vcpkg.exe install glad`

5. <font style="color:rgb(25, 27, 31);">安装glfw库</font>

`<font style="color:rgb(25, 27, 31);">vcpkg.exe install glfw3</font>`

6. 安装 glm库

`vcpkg.exe install glm`

7. <font style="color:rgb(25, 27, 31);">集成到visual studio</font>

`vcpkg.exe integrate install`

---

## 项目环境
Opengl版本 4.2.0

GLSL #420

使用GLFW + GLAD+GLM库

Assimp + stb_image加载贴图和模型

## 主要工作
根据learnopengl教程实现，实现了额外的oit+gpu instance处理半透明物体排序

实测性能可以达到一百万个半透物体500fps左右

