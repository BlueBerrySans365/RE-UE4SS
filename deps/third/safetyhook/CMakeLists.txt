include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

FetchContent_Declare(
    safetyhook
    GIT_REPOSITORY "https://github.com/UE4SS-RE/safetyhook.git"
    GIT_TAG "origin/main"
)
FetchContent_MakeAvailable(safetyhook)
