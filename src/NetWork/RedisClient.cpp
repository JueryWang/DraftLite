#include "NetWork/RedisClient.h"

using namespace std;

void RedisExample()
{
	// 1. 连接 Redis
	redisContext* ctx = redisConnect("127.0.0.1", 6379);
	if (ctx == nullptr || ctx->err) {
		std::cerr << "连接失败: " << (ctx ? ctx->errstr : "内存分配失败") << std::endl;
		return; // 失败直接退出
	}
	
	const char* Channel = "WG MachWiz Channel";
	redisReply* reply = (redisReply*)redisCommand(ctx, "SUBSCRIBE %s", Channel);
	freeReplyObject(reply);

	while (redisGetReply(ctx, (void**)&reply) == REDIS_OK)
	{
		if (reply->type == REDIS_REPLY_ARRAY && reply->elements == 3)
		{
			string type = reply->element[0]->str;
			string chan = reply->element[1]->str;
			string msg = reply->element[2]->str;

			cout << "========================" << endl;
			cout << "content:" << msg << endl;
			cout << "========================" << endl;
		}
		freeReplyObject(reply);
	}

	redisFree(ctx);
}