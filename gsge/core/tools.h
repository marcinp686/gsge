#pragma once

#include <string>
#include <vulkan/vulkan.h>
#include <spdlog/spdlog.h>

#if !defined NDEBUG

struct ErrorDetail
{
    std::string vkResultAsString;
    std::string description;
    bool isCritical{true};
};

extern std::unordered_map<VkResult, ErrorDetail> errorDetails;

#define GSGE_CHECK_RESULT(func)                                                                                                  \
    {                                                                                                                            \
        VkResult result = (func);                                                                                                \
                                                                                                                                 \
        if (result != VK_SUCCESS)                                                                                                \
        {                                                                                                                        \
            if (errorDetails[result].isCritical)                                                                                 \
            {                                                                                                                    \
                SPDLOG_CRITICAL("\x1B[91m{}\033[0m", errorDetails[result].vkResultAsString);                                     \
                SPDLOG_CRITICAL("\t{}, line {}", __FILE__, __LINE__);                                                            \
                SPDLOG_CRITICAL("\t{}", #func);                                                                                  \
                SPDLOG_CRITICAL("\t{}", errorDetails[result].description);                                                       \
                std::terminate();                                                                                                \
            }                                                                                                                    \
            else                                                                                                                 \
            {                                                                                                                    \
                SPDLOG_WARN("\x1B[93m{}\033[0m", errorDetails[result].vkResultAsString);                                         \
                SPDLOG_WARN("\t{}, line {}", __FILE__, __LINE__);                                                                \
                SPDLOG_WARN("\t{}", #func);                                                                                      \
                SPDLOG_WARN("\t{}", errorDetails[result].description);                                                           \
            }                                                                                                                    \
        }                                                                                                                        \
    }

#else
#define GSGE_CHECK_RESULT(func) func

#endif // !NDEBUG
