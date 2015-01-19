

#include <switch.h>
#include "test.h"
#include "aws.h"

/**
 * Test string to sign generation
 */
static void test_string_to_sign(void)
{
	ASSERT_STRING_EQUALS("GET\n\n\nFri, 17 May 2013 19:35:26 GMT\n/rienzo-vault/troporocks.mp3", aws_s3_string_to_sign("GET", "rienzo-vault", "troporocks.mp3", "", "", "Fri, 17 May 2013 19:35:26 GMT"));
	ASSERT_STRING_EQUALS("GET\nc8fdb181845a4ca6b8fec737b3581d76\naudio/mpeg\nThu, 17 Nov 2005 18:49:58 GMT\n/foo/man.chu", aws_s3_string_to_sign("GET", "foo", "man.chu", "audio/mpeg", "c8fdb181845a4ca6b8fec737b3581d76", "Thu, 17 Nov 2005 18:49:58 GMT"));
	ASSERT_STRING_EQUALS("\n\n\n\n//", aws_s3_string_to_sign("", "", "", "", "", ""));
	ASSERT_STRING_EQUALS("\n\n\n\n//", aws_s3_string_to_sign(NULL, NULL, NULL, NULL, NULL, NULL));
	ASSERT_STRING_EQUALS("PUT\n\naudio/wav\nWed, 12 Jun 2013 13:16:58 GMT\n/bucket/voicemails/recording.wav", aws_s3_string_to_sign("PUT", "bucket", "voicemails/recording.wav", "audio/wav", "", "Wed, 12 Jun 2013 13:16:58 GMT"));
}

/**
 * Test signature generation
 */
static void test_signature(void)
{
	char signature[S3_SIGNATURE_LENGTH_MAX];
	signature[0] = '\0';
	ASSERT_STRING_EQUALS("weGrLrc9HDlkYPTepVl0A9VYNlw=", aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, "GET\n\n\nFri, 17 May 2013 19:35:26 GMT\n/rienzo-vault/troporocks.mp3", "hOIZt1oeTX1JzINOMBoKf0BxONRZNQT1J8gIznLx"));
	ASSERT_STRING_EQUALS("jZNOcbfWmD/A/f3hSvVzXZjM2HU=", aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, "PUT\nc8fdb181845a4ca6b8fec737b3581d76\ntext/html\nThu, 17 Nov 2005 18:49:58 GMT\nx-amz-magic:abracadabra\nx-amz-meta-author:foo@bar.com\n/quotes/nelson", "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
	ASSERT_STRING_EQUALS("5m+HAmc5JsrgyDelh9+a2dNrzN8=", aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, "GET\n\n\n\nx-amz-date:Thu, 17 Nov 2005 18:49:58 GMT\nx-amz-magic:abracadabra\n/quotes/nelson", "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
	ASSERT_STRING_EQUALS("OKA87rVp3c4kd59t8D3diFmTfuo=", aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, "", "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
	ASSERT_STRING_EQUALS("OKA87rVp3c4kd59t8D3diFmTfuo=", aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, NULL, "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
	ASSERT_NULL(aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, "GET\n\n\n\nx-amz-date:Thu, 17 Nov 2005 18:49:58 GMT\nx-amz-magic:abracadabra\n/quotes/nelson", ""));
	ASSERT_NULL(aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, "", ""));
	ASSERT_NULL(aws_s3_signature(signature, S3_SIGNATURE_LENGTH_MAX, NULL, NULL));
	ASSERT_NULL(aws_s3_signature(NULL, S3_SIGNATURE_LENGTH_MAX, "PUT\nc8fdb181845a4ca6b8fec737b3581d76\ntext/html\nThu, 17 Nov 2005 18:49:58 GMT\nx-amz-magic:abracadabra\nx-amz-meta-author:foo@bar.com\n/quotes/nelson", "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
	ASSERT_NULL(aws_s3_signature(signature, 0, "PUT\nc8fdb181845a4ca6b8fec737b3581d76\ntext/html\nThu, 17 Nov 2005 18:49:58 GMT\nx-amz-magic:abracadabra\nx-amz-meta-author:foo@bar.com\n/quotes/nelson", "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
	ASSERT_STRING_EQUALS("jZNO", aws_s3_signature(signature, 5, "PUT\nc8fdb181845a4ca6b8fec737b3581d76\ntext/html\nThu, 17 Nov 2005 18:49:58 GMT\nx-amz-magic:abracadabra\nx-amz-meta-author:foo@bar.com\n/quotes/nelson", "OtxrzxIsfpFjA7SwPzILwy8Bw21TLhquhboDYROV"));
}

/**
 * Test amazon URL detection
 */
static void test_check_url(void)
{
	ASSERT_TRUE(aws_s3_is_s3_url("http://bucket.s3-us-west-1.amazonaws.com/object.ext", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("https://bucket.s3-us-west-1.amazonaws.com/object.ext", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("http://bucket.s3.amazonaws.com/object.ext", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("http://bucket.s3.amazonaws.com/object.ext", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("http://bucket.s3.amazonaws.com/object", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("http://red.bucket.s3.amazonaws.com/object.ext", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("https://bucket.s3.amazonaws.com/object.ext", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("https://bucket.s3.amazonaws.com/object", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("https://bucket.s3.amazonaws.com/recordings/1240fwjf8we.mp3", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("https://bucket.s3.amazonaws.com/en/us/8000/1232345.mp3", NULL));
	ASSERT_TRUE(aws_s3_is_s3_url("https://bucket_with_underscore.s3.amazonaws.com/en/us/8000/1232345.mp3", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("bucket.s3.amazonaws.com/object.ext", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("https://s3.amazonaws.com/bucket/object", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("http://s3.amazonaws.com/bucket/object", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("http://google.com/", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("http://phono.com/audio/troporocks.mp3", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("", NULL));
	ASSERT_FALSE(aws_s3_is_s3_url(NULL, NULL));
	ASSERT_FALSE(aws_s3_is_s3_url("https://example.com/bucket/object", "example.com"));
	ASSERT_TRUE(aws_s3_is_s3_url("http://bucket.example.com/object", "example.com"));
	ASSERT_FALSE(aws_s3_is_s3_url("", "example.com"));
	ASSERT_FALSE(aws_s3_is_s3_url(NULL, "example.com"));
}

/**
 * Test bucket/object extraction from URL
 */
static void test_parse_url(void)
{
	char *bucket;
	char *object;
	aws_s3_parse_url(strdup("http://quotes.s3.amazonaws.com/nelson"), NULL, &bucket, &object);
	ASSERT_STRING_EQUALS("quotes", bucket);
	ASSERT_STRING_EQUALS("nelson", object);

	aws_s3_parse_url(strdup("https://quotes.s3.amazonaws.com/nelson.mp3"), NULL, &bucket, &object);
	ASSERT_STRING_EQUALS("quotes", bucket);
	ASSERT_STRING_EQUALS("nelson.mp3", object);

	aws_s3_parse_url(strdup("http://s3.amazonaws.com/quotes/nelson"), NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(strdup("http://quotes/quotes/nelson"), NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(strdup("http://quotes.s3.amazonaws.com/"), NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(strdup("http://quotes.s3.amazonaws.com"), NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(strdup("http://quotes"), NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(strdup(""), NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(NULL, NULL, &bucket, &object);
	ASSERT_NULL(bucket);
	ASSERT_NULL(object);

	aws_s3_parse_url(strdup("http://bucket.s3.amazonaws.com/voicemails/recording.wav"), NULL, &bucket, &object);
	ASSERT_STRING_EQUALS("bucket", bucket);
	ASSERT_STRING_EQUALS("voicemails/recording.wav", object);

	aws_s3_parse_url(strdup("https://my-bucket-with-dash.s3-us-west-2.amazonaws.com/greeting/file/1002/Lumino.mp3"), NULL, &bucket, &object);
	ASSERT_STRING_EQUALS("my-bucket-with-dash", bucket);
	ASSERT_STRING_EQUALS("greeting/file/1002/Lumino.mp3", object);
	
	aws_s3_parse_url(strdup("http://quotes.s3.foo.bar.s3.amazonaws.com/greeting/file/1002/Lumino.mp3"), NULL, &bucket, &object);
	ASSERT_STRING_EQUALS("quotes.s3.foo.bar", bucket);
	ASSERT_STRING_EQUALS("greeting/file/1002/Lumino.mp3", object);

	aws_s3_parse_url(strdup("http://quotes.s3.foo.bar.example.com/greeting/file/1002/Lumino.mp3"), "example.com", &bucket, &object);
	ASSERT_STRING_EQUALS("quotes.s3.foo.bar", bucket);
	ASSERT_STRING_EQUALS("greeting/file/1002/Lumino.mp3", object);
}

/**
 * Test Authorization header creation
 */
static void test_authorization_header(void)
{
	ASSERT_STRING_EQUALS("AWS AKIAIOSFODNN7EXAMPLE:YJkomOaqUJlvEluDq4fpusID38Y=", aws_s3_authentication_create("GET", "https://vault.s3.amazonaws.com/awesome.mp3", NULL, "audio/mpeg", "", "AKIAIOSFODNN7EXAMPLE", "0123456789012345678901234567890123456789", "1234567890"));
	ASSERT_STRING_EQUALS("AWS AKIAIOSFODNN7EXAMPLE:YJkomOaqUJlvEluDq4fpusID38Y=", aws_s3_authentication_create("GET", "https://vault.s3.amazonaws.com/awesome.mp3", "s3.amazonaws.com", "audio/mpeg", "", "AKIAIOSFODNN7EXAMPLE", "0123456789012345678901234567890123456789", "1234567890"));
	ASSERT_STRING_EQUALS("AWS AKIAIOSFODNN7EXAMPLE:YJkomOaqUJlvEluDq4fpusID38Y=", aws_s3_authentication_create("GET", "https://vault.example.com/awesome.mp3", "example.com", "audio/mpeg", "", "AKIAIOSFODNN7EXAMPLE", "0123456789012345678901234567890123456789", "1234567890"));
}

/**
 * Test pre-signed URL creation
 */
static void test_presigned_url(void)
{
	ASSERT_STRING_EQUALS("https://vault.s3.amazonaws.com/awesome.mp3?Signature=YJkomOaqUJlvEluDq4fpusID38Y%3D&Expires=1234567890&AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE", aws_s3_presigned_url_create("GET", "https://vault.s3.amazonaws.com/awesome.mp3", NULL, "audio/mpeg", "", "AKIAIOSFODNN7EXAMPLE", "0123456789012345678901234567890123456789", "1234567890"));
	ASSERT_STRING_EQUALS("https://vault.s3.amazonaws.com/awesome.mp3?Signature=YJkomOaqUJlvEluDq4fpusID38Y%3D&Expires=1234567890&AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE", aws_s3_presigned_url_create("GET", "https://vault.s3.amazonaws.com/awesome.mp3", "s3.amazonaws.com", "audio/mpeg", "", "AKIAIOSFODNN7EXAMPLE", "0123456789012345678901234567890123456789", "1234567890"));
	ASSERT_STRING_EQUALS("https://vault.example.com/awesome.mp3?Signature=YJkomOaqUJlvEluDq4fpusID38Y%3D&Expires=1234567890&AWSAccessKeyId=AKIAIOSFODNN7EXAMPLE", aws_s3_presigned_url_create("GET", "https://vault.example.com/awesome.mp3", "example.com", "audio/mpeg", "", "AKIAIOSFODNN7EXAMPLE", "0123456789012345678901234567890123456789", "1234567890"));
}

/**
 * main program
 */
int main(int argc, char **argv)
{
	TEST_INIT
	TEST(test_string_to_sign);
	TEST(test_signature);
	TEST(test_check_url);
	TEST(test_parse_url);
	TEST(test_authorization_header);
	TEST(test_presigned_url);
	return 0;
}
