---
marp: true
theme: gaia
---

<style>
section {
  background-image: url('./_static/background.png');
  background-size: cover;
  background-position: center;
  font-size: 200%;
}
</style>

<!-- slide: title -->

<!-- _class: lead -->

# 如何简单的在项目中添加 CI

---

## 什么是 CI

一个 CI 可以看成是一个流水线，包括：

- 工作，定义做什么。例如，编译或测试代码的作业
- 阶段，定义何时运行作业。例如，编译后进行测试，测试完进行发布


### 为什么需要这么复杂的 CI?

- 更高效的工作
- 更有助于提早发现问题

---

### 在一个仓库中，往往需要哪些工作？

- 检查 README 链接 

- 编译 example
- 编译文档
- 硬件测试固件
- 发布

```
stages:
  - pre_check   #预检查
  - build       #编译 example, components test
  - target_test #硬件测试
  - build_doc   #编译文档
  - deploy      #发布
```
---

### 谁来执行 CI job？

- CI 的 job 是通过 tag 指定给对应的 `runner` 运行。
  
  ![runner](./_static/runner.png)

---

### 如何搭建 runner？

1. 使用已经创建好的共用 runner。比如说专门用来编译 example 的 build runner。专门用于 esp-docs 编译的 runner。

    可以联系 `@XiaoXuFeng`, `@FuHanxi`

---

2. 搭建自己的 runner，适用于特殊需求的 runner，如挂载特殊的开发板等等
   - 准备:
        - 一个树莓派(64G 内存卡) / linux 电脑
        - 可以接入内网的网络环境
   - 步骤，具体参考[部署 runner](https://gitlab.espressif.cn:6688/ae_group/esp-iot-solution/-/wikis/esp-iot-solution-add-ci-&-target_test#%E9%83%A8%E7%BD%B2-runner)：
        - 刷入 linux 系统
        - 使用 [test-runner-management](https://gitlab.espressif.cn:6688/qa/test-runner-management) 构建 runner & docker 环境
        - 在仓库 runner 管理界面启用 runner 并添加相关 tag
            ![runner](./_static/runner.png)

---

## Stage: pre_check

esp-iot-solution 中用于检查 README 链接的 CI

```
check_readme_links:
  stage: pre_check
  image: python:3.9
  tags: ["build", "amd64", "internet"]
  allow_failure: true
  script:
    - python ${CI_PROJECT_DIR}/tools/ci/check_readme_links.py
```

![width:300px bg right:25%](./_static/precheck.png)

---

## Stage: build

[`idf-build-apps`](https://docs.espressif.com/projects/idf-build-apps/en/latest/index.html) 是一款可以帮助用户更快查找和构建 ESP-IDF 项目的工具

包括：
* find: 查找路径下所有可用于编译的应用程序
```
idf-build-apps find -p . --recursive --target esp32
```

* build: 编译路径下的所有程序
```
idf-build-apps build -p . --recursive --target esp32 --dry-run -vv
```

---

### 最简单的编译所有 example 的方式

如果只是想快速的编译所有的 example

```
build_apps:
  stage: build
  image: espressif/idf:release-v5.0
  tags: build
  script:
    - pip install --upgrade idf-component-manager
    - pip install idf_build_apps
    - idf-build-apps find -p . --recursive --target esp32 -vvvv
    - idf-build-apps build -p . --recursive --target esp32 -vvvv
```

---

* `idf-build-apps find -p . --recursive --target esp32 -vvvv`
log:
(cmake) App ./examples/screen, target esp32, sdkconfig (default), build in esp-iot-solution/examples/screen/build

---

### 如何在不同 IDF 版本下进行编译？

使用 matrix 生成两个子任务，注意 image 关键字不可以在 matrix 中被设置，但是可以通过变量的方式传递

```
build_apps:
  stage: build
  parallel:
    matrix:
      - IMAGE: espressif/idf:release-v4.4
      - IMAGE: espressif/idf:release-v5.0
  image: ${IMAGE}
  tags: build
  script:
    - pip install --upgrade idf-component-manager
    - pip install idf_build_apps
    - idf-build-apps find -p . --recursive --target esp32 -vvvv
    - idf-build-apps build -p . --recursive --target esp32 -vvvv
```

---

一个任务会被拆分成两个子任务

![width:1000px](./_static/matrix.png)

---

### .build-rules.yml

* 该文件用于指定 example 编译的条件

  ```
  example/hello_world:
    enable:
      - if: INCLUDE_DEFAULT == 1
      - if: SOC_CPU_CORES_NUM > 1
    
    disable:
      - if: IDF_TARGET == "esp32c2"
        temporary: true
        reason: target esp32c2 is not supported yet
  ```

* 注意对于 `a and b and c` 的运算是不支持的, 只支持 `(a and b) and c`
* 需要在 `manifest_files` 中指定 `.build-rules.yml` 路径，支持多个 `.build-rules.yml` 文件, 上下级文件不会继承。

---

### 如何只编译变动的工程？

通过添加 rules 指定 job 的工作范围，比如我们只想在 `examples` 目录有改动的时候执行 `build_apps` 任务

```
.if-dev-push: &if-dev-push
  if: '$CI_COMMIT_REF_NAME != "master" 
  && $CI_COMMIT_BRANCH !~ /^release\/v/ 
  && $CI_COMMIT_TAG !~ /^v\d+\.\d+(\.\d+)?($|-)/ 
  && ($CI_PIPELINE_SOURCE == "push" 
  || $CI_PIPELINE_SOURCE == "merge_request_event")'

.patterns-examples: &patterns-examples
  - "examples/**/*"

.rules:build:examples:
  rules:
    - <<: *if-dev-push
      changes: *patterns-examples

build_apps:
  extends:
    .rules:build:examples
  stage: build
  parallel:
    matrix:
      - IMAGE: espressif/idf:release-v4.4
      - IMAGE: espressif/idf:release-v5.0
  image: ${IMAGE}
  tags: build
  script:
    - pip install --upgrade idf-component-manager
    - pip install idf_build_apps
    - idf-build-apps find
    - idf-build-apps build

```

---

### 如何将编译的产物上传？

`artifacts` 用于描述产物是否上传，上传哪些产物，等等。

```
.build_examples_template: &build_examples_template
  stage: build
  artifacts:
    when: always  #on_failure
    path:
      - "**/build*/*.bin"
    expire_in:
      1 week
```

---

通过增加 `artifacts` 可以让 runner 将运行的产物上传至服务器，例如

```
build_apps:
  extends:
    .rules:build:examples      # 控制只在 examples 目录内容有改动时运行该 job
    .build_examples_template   # 上传编译得到的 bin 文件
  ...
```

![width:500px](./_static/job_artifacts.png)

---

### 如何拉取生成的产物？

可以使用 `needs` 关键字得到指定 job 编译出来的产物。同时 `needs` 还可以定义任务之间的依赖关系，意味着一个任务必须在另一个任务之前执行。

```
examples_test:
  extends:
    .rules:build:examples      # 控制只在 examples 目录内容有改动时运行该 job
  needs:
    - job: "build_apps"
      artifacts: true          # 是否需要产物
      optional: true           # 所需任务不是必须项，即失败了也可以继续执行该 job
  ···
```

---

### 如何简单的为工程添加硬件测试？

使用以下工具可以很好的帮助我们自动化硬件测试

* [unit_test](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32/api-guides/unit-tests.html) ESP32 中的单元测试

* [pytest](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/contribute/esp-idf-tests-with-pytest.html) 提供一些列服务：烧录固件/串口监控...

---

#### unit_test

1. 用于生成测试用例，这部分请[参考](https://github.com/espressif/esp-iot-solution/tree/master/components/usb/usb_stream/test_apps)
2. 可以检测内存泄露，需要在 tear_down 和 tear_up 的 hook 函数中添加相关检查代码。
3. 通过指定 tag 来运行指定的 test_case, `*` 运行全部

```
TEST_CASE("test uac mic", "[devkit][uac][mic]")
{
  ...
}
```

4. 建议将每一个 test_apps 作为完整工程放在组件或者例程下。

---

#### 如何在仓库中添加 pytest

1. 添加 [conftest.py](https://github.com/espressif/esp-iot-solution/blob/master/conftest.py) 文件。
    * 定义全局的 `fixtures`, fixture是在测试函数运行前后，由pytest执行的外壳函数。 如果fixture函数包含yield，那么系统会在yield处停止，转而运行测试函数，等测试函数执行完毕后再回到fixture，继续执行yield后面的代码。
    ```
    @pytest.fixture()
    def some_data():
        return 42

    def test_some_data(some_data):
        assert some_data == 42
    ```

---

2. 添加 [pytest.ini](https://github.com/espressif/esp-iot-solution/blob/master/pytest.ini)文件
    * 用于配置 `pytest` 行为, 相当于 pytest 的 `sdkconfig.defaults`
    * 设置测试运行的过滤规则
    * 定义自定义的命令行，比如可以增加 `target`, `env` 等等

---

#### 简单的 pytest 测试用例怎么写？

我们在 `esp_lcd_panel_io_additions` 中设置了 `python_files = pytest_*.py`, 这条命令规定了可以被识别到的 pytest 工程。

例子：在 `usb_stream` 中的 [`pytest_esp_lcd_panel_io_addtions.py`](https://github.com/espressif/esp-iot-solution/blob/master/components/display/lcd/esp_lcd_panel_io_additions/test_apps/pytest_esp_lcd_panel_io_addtions.py)

```
@pytest.mark.target('esp32s3')             # 限定了 target 必须为 esp32s3
@pytest.mark.env('esp32_s3_lcd_ev_board')  # 限定了目前的编译环境必须为 esp32_s3_lcd_ev_board
def test_usb_stream(dut: Dut)-> None:
    dut.run_all_single_board_cases()       # 获取 unit_test 的运行结果(全运行)
```

如何触发上面的任务？

```
pytest components/display/lcd/esp_lcd_panel_io_additions --target esp32s2 --env esp32_s3_lcd_ev_board
```

`env` 一般用来指定硬件环境，可以自行设置任意值

---

#### pytest 的流程是什么样的？

1. 首先需要 build job 将需要进行硬件测试的 bin 编译出来
2. 通过拉取产物的方式，将 bin 拉取下来
3. 通过命令行指定 `target`, `app_path`, `env`, `defaults` 来指定运行 pytest 测试用例，并为测试用例找到适用的 bin 文件。
比如设定 `--target esp32s3 --defaults quick`
    ```
    build_esp32s2_defaults   x
    build_esp32s3_defaults   x
    build_esp32s2_quick      x
    build_esp32s3_quick      √
    ```
4. 下载固件进开发板，通过串口进行交互，自动运行 pytest 脚本 

---

#### 注意事项

* pytest 如何识别到具体将固件通过哪一个串口进行烧录?
pytest 采用 esptool.py 进行芯片识别，如果同时挂载两个 esp32s3 的开发板，那么串口的选取则是随机的。可以通过 parametrize 指定端口
  ```
  @pytest.mark.parametrize(
          'port',['dev/ttyUSB0']
  )
  ```

* 挂载多个开发板可以提高效率吗？
pytest 一次只能执行一个 test_case

---

## Stage: build_docs

使用 `esp-docs` 框架编译文档，好处

* 打包好的 python 环境，免去本地安装环境问题
* 自带如报错检查，中英文链接跳转的脚本功能

```
├── requirements.txt           #定义依赖项
├── en                         #存放英文相关文档
│   ├── conf.py                #英文文档的配置文件
│   └── index.rst              #索引文件，入口处
├── zh_CN
│   ├── conf.py
│   └── index.rst
├── conf_common.py             #公共的配置文件，用于定义适用于所有语言版本的共享配置和选项
└── sphinx-known-warnings.txt  #记录 Sphinx 构建过程中产生的已知警告（known warnings）
```

---

### 如何简单的创建 rst 编译的 job

```
.build_docs_template:
  image: $CI_DOCKER_REGISTRY/esp-idf-doc-env-v5.2:1-1
  stage: build_doc
  tags:
    - build_docs
  artifacts:
    when: always
    paths:
      - docs/_build/*/*/*.txt
      - docs/_build/*/*/html/*
  script:
    - cd docs
    - python -m pip install -r requirements.txt 
    - build-docs -t esp32 -bs html -l en build
    - build-docs -t esp32 -bs html -l zh_CN build
    - build-docs -t esp32 -bs latex -l en build
    - build-docs -t esp32 -bs latex -l zh_CN build
    - ./check_lang_folder_sync.sh
```

---

### 预览编译出来的 html 文件

1. 本地预览 html 文件，优点是便捷方便，缺点是没法中英文一起预览，互相跳转。本地链接：http://localhost:8000/ 

```
python3 -m http.server 8000 --directory _build/zh_CN/esp32/html
```

---

2. 服务器上面 `preview`, 优点是和正式发布的文档一样，缺点是必须要 CI 成功部署。

```
deploy_docs_preview:
  extends:
    - .deploy_docs_template
    - .rules:deploy:docs_preview
  variables:
    TYPE: "preview"
    DOCS_BUILD_DIR: "${CI_PROJECT_DIR}/docs/_build/"
    DOCS_DEPLOY_PRIVATEKEY: "$DOCS_PREVIEW_DEPLOY_KEY"
    DOCS_DEPLOY_SERVER: "$DOCS_PREVIEW_SERVER"
    DOCS_DEPLOY_SERVER_USER: "$DOCS_PREVIEW_SERVER_USER"
    DOCS_DEPLOY_PATH: "$DOCS_PREVIEW_PATH"
    DOCS_DEPLOY_URL_BASE: "$DOCS_PREVIEW_URL_BASE"
```

---

## 仓库自动检查机器人： bot

[bot](https://qa.gitlab-pages.espressif.cn:6688/bot/) 是一个可以自动执行一些任务的机器人，你可以
* 使用它主动的打开一些 CI job
* 帮助你 rebase master
* 检查代码规范
* 自动重试失败的任务

  ![width:600px](./_static/bot_rebase.png)

---

## Future

* 自动发布 example 固件到 [launchpad](https://espressif.github.io/esp-launchpad/)
* 更完善的测试报告，自动化生成芯片的各类参数，吞吐量，刷屏速率
* 自动 review 代码
* ...

---

## 参考链接

* pytest docs: [https://learning-pytest.readthedocs.io/zh/latest/doc/test-function/mark.html](https://learning-pytest.readthedocs.io/zh/latest/doc/test-function/mark.html)

* runner docs: [https://gitlab.espressif.cn:6688/qa/qa-group-wiki](https://gitlab.espressif.cn:6688/qa/qa-group-wiki)

* idf-build-apps
  * [https://github.com/espressif/idf-build-apps](https://github.com/espressif/idf-build-apps)
  * [https://docs.espressif.com/projects/idf-build-apps/en/latest/index.html](https://docs.espressif.com/projects/idf-build-apps/en/latest/index.html)

---

* pytest-embedded
  * [https://github.com/espressif/pytest-embedded](https://github.com/espressif/pytest-embedded)
  * [https://docs.espressif.com/projects/pytest-embedded/en/latest/](https://docs.espressif.com/projects/pytest-embedded/en/latest/)
  * [ESP-IDF pytest](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/contribute/esp-idf-tests-with-pytest.html)

* CI-templates: [https://gitlab.espressif.cn:6688/fuhanxi/ci-templates](https://gitlab.espressif.cn:6688/fuhanxi/ci-templates)

* CI 培训视频：
  * [第一期 硬件测试](https://espressifsystems.sharepoint.com/sites/softwareplatformschannel/Shared%20Documents/Forms/AllItems.aspx?RootFolder=%2Fsites%2Fsoftwareplatformschannel%2FShared%20Documents%2FTrainings%2FCN&FolderCTID=0x01200097835F821BA18D4E8128CDE28F21251C&OR=Teams%2DHL&CT=1681201880229&clickparams=eyJBcHBOYW1lIjoiVGVhbXMtRGVza3RvcCIsIkFwcFZlcnNpb24iOiIyNy8yMzAzMDUwMTExMCIsIkhhc0ZlZGVyYXRlZFVzZXIiOmZhbHNlfQ%3D%3D)
  * [第二期 New about CI and Services](https://espressifsystems-my.sharepoint.com/:v:/g/personal/sergei_silnov_espressif_com/ERFofN3xjJNOlVf8UqlTx6UBqJPxr0ImRt4_-bQIah_HWA)

* [esp-iot-solution 如何添加 CI](https://gitlab.espressif.cn:6688/ae_group/esp-iot-solution/-/wikis/esp-iot-solution-add-ci-&-target_test)