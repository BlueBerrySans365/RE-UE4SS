include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    safetyhook
    GIT_REPOSITORY "https://github.com/BlueBerrySans365/safetyhook.git"
    GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(safetyhook)
