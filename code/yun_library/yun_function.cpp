////////////////////////////////////////////////////////
//  The function "generate_key()" is revised from the code from 
//  https://www.codepool.biz/how-to-use-openssl-generate-rsa-keys-cc.html
////////////////////////////////////////////////////////

#include "yun_function.h"

const BYTE* int_to_header(unsigned data_size)
{
    unsigned size = data_size;
    char tag = '\0';
    BYTE* sizeInfo;//In sizeInfo, 1 byte for tag, and the other is data_size
    int infoSize = 0;
    if(size < TAG_B_CAPACITY)
    {
        tag = 'B';
        sizeInfo = new BYTE[TAG_B_SIZE+1];
        infoSize = TAG_B_SIZE;
    }
    else if(size < TAG_H_CAPACITY)
    {
        tag = 'H';
        sizeInfo = new BYTE[TAG_H_SIZE+1];
        infoSize = TAG_H_SIZE;
    }
    else if(size < TAG_L_CAPACITY)
    {
        tag = 'L';
        sizeInfo = new BYTE[TAG_L_SIZE+1];
        infoSize = TAG_L_SIZE;
    }

    sizeInfo[0] = tag;
    unsigned leftSize = size;
    for(int a = infoSize; a >= 1; a--)
    {
        sizeInfo[a] = (BYTE)leftSize;
        leftSize >>= 8;
    }

// The following code is used to debug .
//    for(int a = 0; a < infoSize+1; a ++)
//    {
//    	if(a == 0)
//    		cout << sizeInfo[a] << endl;
//    	else
//    		printf("%x\n", sizeInfo[a]);
// 	}
// 	cout << "test ends" << endl;
// 	cout << "debug: " << sizeInfo << endl;
//     int sizeField = getTagSize(tag);
//     for(int a = 1; a <= sizeField; a++)
//     {
//         char pre_msg_size[4] = {'\0'};
//         pre_msg_size[0] = sizeInfo[a];
//         unsigned msg_size = *((unsigned*)pre_msg_size);
//         cout << msg_size;
//     }
//     cout << endl;
	return sizeInfo;
}

int getTagSize(const char& tag)
{
    if(tag == 'B')
        return TAG_B_SIZE;
    else if(tag == 'H')
        return TAG_H_SIZE;
    else if(tag == 'L')
        return TAG_L_SIZE;
    else
        throw runtime_error("[!] Failed to get information of send_msg's header. \n");
}


int yun_send(int socket_fd, const char* send_msg, int msg_size)
/*
@return 0 when success;
@return 1 when failure;
*/
/*
The reason that I do not use string to hanle send_msg is that it is likely that
 \x00 occurs in the send_msg. If using string as handler, send_msg would send 
 data imcompletely. 
For example: 
    If the msg_size is 0x2000, then the size field in header would be "H\x20\x00". 
    In this situation, using handler of string type would trigger error because the 
    data following \x00 would not be sent. 
*/
{
    if(msg_size < 0)
    {
        throw invalid_argument(string("[!] The \"msg_send\" argument of function \"yun_send\"") +
                                      "should be an positive integer. ");
    }
    const BYTE* header = int_to_header(msg_size);
    char tag = header[0];
    int headerSize = 1 + getTagSize(tag); // 1 is the size of tag

    int dataSize = headerSize + msg_size;
    char send_data[dataSize];
    /*This part I choose memcpy instead of strcpy owing to the potential failure triggered by \x00. */
    memcpy(send_data, header, headerSize);
    memcpy((send_data+headerSize), send_msg, msg_size);
    
    // Debug starts
    // cout << "Debug Yun_send: " << endl;
    // cout << send_data[0];
    // int sizeField = getTagSize(tag);
    // for(int a = 1; a <= sizeField; a++)
    // {
    //     char pre_msg_size[4] = {'\0'};
    //     pre_msg_size[0] = send_data[a];
    //     unsigned msg_size = *((unsigned*)pre_msg_size);
    //     cout << msg_size;
    // }
    // for(int a = 1+sizeField; a < dataSize; a++)
    // {
    //     cout << send_data[a];
    // }
    // cout << endl;
    // Debug ends

    char *ptr = send_data;
    int bytes_left = dataSize;

    while(bytes_left > 0)
    {
        int sendSize = bytes_left<BUFFER_SIZE?bytes_left:BUFFER_SIZE;
        int written_bytes = send(socket_fd, ptr, sendSize, 0);
        if(written_bytes <= 0)
        {       
            if(errno == EINTR)
                written_bytes=0;
            else             
                return(-1);
        }
        bytes_left -= written_bytes;
        ptr += written_bytes;     
    }
    return(0);
}

const char* yun_recv(int cli_sock)
/*
@return the data recv
*/
{
    char recvBuffer[BUFFER_SIZE];
    memset(recvBuffer, '\0', BUFFER_SIZE);
    char* processPtr = recvBuffer;// the ptr pointing to the position where we finish processing data
    char* recvPtr = recvBuffer;   // the ptr pointing to the position where we recv data
    int totalRecvNum = 0; 
    int recvByteNum = 0;
    char tag = '\0';

    // recv tags    
    do{
        recvByteNum = recv(cli_sock, recvBuffer, BUFFER_SIZE, 0);
        recvPtr += recvByteNum;
        totalRecvNum += recvByteNum;
    }
    while(recvByteNum <= 1);

    tag = recvBuffer[0];
    int sizeFieldlen = getTagSize(tag);
    processPtr += 1;

    // recv sizeField
    while(totalRecvNum < sizeFieldlen+1)// plus 1 owing to tag size
    {
        recvByteNum = recv(cli_sock, recvPtr, BUFFER_SIZE-totalRecvNum, 0);
        recvPtr += recvByteNum;
        totalRecvNum += recvByteNum;
    }

    char pre_msg_size[4] = {'\0'};
    int fillPtr = 0;
    for(int a = sizeFieldlen-1; a >= 0; a--)
    {
        pre_msg_size[fillPtr] = processPtr[a];
        fillPtr++;
    }
    unsigned msg_size = *((unsigned*)pre_msg_size);

    processPtr += sizeFieldlen;

    char* msg = new char[msg_size + 1];

    memset(msg, '\0', msg_size + 1);
    // recv data
    int alreadyRecvMsgLen = 0;
    if(recvPtr > processPtr)
    {
        memcpy(msg, processPtr, (recvPtr-processPtr));
        alreadyRecvMsgLen += (recvPtr-processPtr);
        processPtr = recvPtr;
    }

    while(alreadyRecvMsgLen < msg_size)
    {
        recvByteNum = recv(cli_sock, recvBuffer, BUFFER_SIZE, 0);
        memcpy((msg+alreadyRecvMsgLen), recvBuffer, recvByteNum);
        alreadyRecvMsgLen += recvByteNum;
        totalRecvNum += recvByteNum;
    }
    // cout << "buffer_size: " << msg_size << endl;
    // cout << "totalRecv: " << totalRecvNum << endl;
    // cout << "test->" << msg <<"<-test" << endl;
    return msg;
}

int yun_send(int socket_fd, string send_msg)
{
    return yun_send(socket_fd, send_msg.c_str(), strlen(send_msg.c_str()));
}


inline void free_all(BIO* bp_public, BIO* bp_private, RSA* r, BIGNUM* bne)
{
	BIO_free_all(bp_public);
	BIO_free_all(bp_private);
	RSA_free(r);
	BN_free(bne);
}

bool generate_key(string key_owner, fs::path public_key_directory, fs::path private_key_directory)
{
	int				ret = 0;
	RSA				*r = NULL;
	BIGNUM			*bne = NULL;
	BIO				*bp_public = NULL, *bp_private = NULL;

	int				bits = 2048;
	unsigned long	e = RSA_F4;

	// 1. generate rsa key
	bne = BN_new();
	ret = BN_set_word(bne,e);
	if(ret != 1){
		free_all(bp_public, bp_private, r, bne);
        return false;
	}

	r = RSA_new();
	ret = RSA_generate_key_ex(r, bits, bne, NULL);
	if(ret != 1){
		free_all(bp_public, bp_private, r, bne);
        return false;
	}

	// 2. save public key
    if(not fs::is_directory(public_key_directory))
    {
        free_all(bp_public, bp_private, r, bne);
        throw runtime_error("[!] argument \"private_key_directory\" is not a directory.\n");
        return false;
    }

    string public_file_name = key_owner + "_public.pem";
    fs::path public_dir (public_key_directory);
    fs::path public_file (public_file_name.c_str());
    fs::path public_key_path = public_dir / public_file;

	bp_public = BIO_new_file(public_key_path.c_str(), "w+");
	ret = PEM_write_bio_RSAPublicKey(bp_public, r);

	if(ret != 1){
		free_all(bp_public, bp_private, r, bne);
        return false;
	}

	// 3. save private key
    if(not fs::is_directory(private_key_directory))
    {
        free_all(bp_public, bp_private, r, bne);
        throw runtime_error("[!] argument \"private_key_directory\" is not a directory.\n");
        return false;
    }

    string private_file_name = key_owner + "_private.pem";
    fs::path private_dir (private_key_directory);
    fs::path private_file (private_file_name.c_str());
    fs::path private_key_path = private_dir / private_file;
    
	bp_private = BIO_new_file(private_key_path.c_str(), "w+");
	ret = PEM_write_bio_RSAPrivateKey(bp_private, r, NULL, NULL, 0, NULL, NULL);

	if(ret != 1){
		free_all(bp_public, bp_private, r, bne);
        return false;
	}

    free_all(bp_public, bp_private, r, bne);
	return (ret == 1);
}

void directory_check_create(fs::path target)
{
    if(!fs::is_directory(target))
    {
        if(!fs::create_directory(target))
        {
            string error_msg = "[!] There is no directory named \"";
            error_msg += target.c_str();
            error_msg += "\", and you have no permission to create one. \n";
            throw runtime_error(error_msg.c_str());
        }
    }
}

const char* getListInfo(int cli_sock)
{
    yun_send(cli_sock, (string(LIST)+'\n').c_str(), strlen(LIST)+1);
    return yun_recv(cli_sock);
}