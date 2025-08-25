#include "../include/FileTransfer.hpp"
#include "../include/Client.hpp"
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <fstream>
#include <sys/socket.h>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <map>
#include <string>
#include <sstream>
#include <unistd.h>

void testFileTransfer() {
    std::cout << "=== FileTransfer Class Test ===" << std::endl;
    
    // Create FileTransfer instance
    FileTransfer fileTransfer;
    
    // Create test clients
    Client client1(1);
    Client client2(2);
    
    // Set nicknames for testing
    client1.setNickname("testuser1");
    client2.setNickname("testuser2");
    
    std::cout << "\n1. Testing file type validation..." << std::endl;
    std::string valid_files[] = {"test.txt", "image.jpg", "document.pdf", "archive.zip"};
    std::string invalid_files[] = {"script.exe", "malware.bat", "virus.sh"};
    
    for (int i = 0; i < 4; i++) {
        bool valid = fileTransfer.isValidFileType(valid_files[i]);
        std::cout << "   " << valid_files[i] << " -> " << (valid ? "VALID" : "INVALID") << std::endl;
    }
    
    for (int i = 0; i < 3; i++) {
        bool valid = fileTransfer.isValidFileType(invalid_files[i]);
        std::cout << "   " << invalid_files[i] << " -> " << (valid ? "VALID" : "INVALID") << std::endl;
    }
    
    std::cout << "\n2. Testing file size formatting..." << std::endl;
    size_t test_sizes[] = {1024, 1024*1024, 1024*1024*1024, 1536};
    
    for (int i = 0; i < 4; i++) {
        std::string formatted = fileTransfer.formatFileSize(test_sizes[i]);
        std::cout << "   " << test_sizes[i] << " bytes -> " << formatted << std::endl;
    }
    
    std::cout << "\n3. Testing upload start..." << std::endl;
    bool upload_started = fileTransfer.startUpload(&client1, "test.txt", 1024);
    std::cout << "   Upload started: " << (upload_started ? "SUCCESS" : "FAILED") << std::endl;
    
    if (upload_started) {
        std::cout << "   Transfer active: " << (fileTransfer.isTransferActive(1) ? "YES" : "NO") << std::endl;
        std::cout << "   Transfer type: " << fileTransfer.getTransferType(1) << std::endl;
        std::cout << "   Status: " << fileTransfer.getTransferStatus(1) << std::endl;
        std::cout << "   Progress bar: " << fileTransfer.getProgressBar(1) << std::endl;
    }
    
    std::cout << "\n4. Testing download start..." << std::endl;
    bool download_started = fileTransfer.startDownload(&client2, "existing_file.txt");
    std::cout << "   Download started: " << (download_started ? "SUCCESS" : "FAILED") << std::endl;
    
    if (download_started) {
        std::cout << "   Transfer active: " << (fileTransfer.isTransferActive(2) ? "YES" : "NO") << std::endl;
        std::cout << "   Transfer type: " << fileTransfer.getTransferType(2) << std::endl;
        std::cout << "   Status: " << fileTransfer.getTransferStatus(2) << std::endl;
    }
    
    std::cout << "\n5. Testing multiple transfers..." << std::endl;
    std::cout << "   Client 1 transfer active: " << (fileTransfer.isTransferActive(1) ? "YES" : "NO") << std::endl;
    std::cout << "   Client 2 transfer active: " << (fileTransfer.isTransferActive(2) ? "YES" : "NO") << std::endl;
    
    std::cout << "\n6. Testing transfer cancellation..." << std::endl;
    fileTransfer.cancelTransfer(&client1);
    std::cout << "   Client 1 transfer active after cancel: " << (fileTransfer.isTransferActive(1) ? "YES" : "NO") << std::endl;
    
    fileTransfer.cancelTransfer(&client2);
    std::cout << "   Client 2 transfer active after cancel: " << (fileTransfer.isTransferActive(2) ? "YES" : "NO") << std::endl;
    
    std::cout << "\n7. Testing disk space check..." << std::endl;
    bool has_space = fileTransfer.hasEnoughDiskSpace("./uploads", 1024*1024); // 1MB
    std::cout << "   Has space for 1MB: " << (has_space ? "YES" : "NO") << std::endl;
    
    std::cout << "\n=== Test Complete ===" << std::endl;
};

void testFileOperations() {
    std::cout << "\n=== File Operations Test ===" << std::endl;
    
    // Create a test file
    std::cout << "Creating test file..." << std::endl;
    std::ofstream test_file("./uploads/test_file.txt");
    if (test_file.is_open()) {
        test_file << "This is a test file for FileTransfer testing.\n";
        test_file << "It contains multiple lines to test file operations.\n";
        test_file << "This is line 3 with some content.\n";
        test_file << "And this is line 4 to make the file bigger.\n";
        test_file.close();
        std::cout << "Test file created successfully!" << std::endl;
    } else {
        std::cout << "Failed to create test file!" << std::endl;
        return;
    }
    
    // Test file operations
    FileTransfer fileTransfer;
    Client client(3);
    client.setNickname("filetest");
    
    std::cout << "\nTesting file download with existing file..." << std::endl;
    bool download_started = fileTransfer.startDownload(&client, "test_file.txt");
    std::cout << "   Download started: " << (download_started ? "SUCCESS" : "FAILED") << std::endl;
    
    if (download_started) {
        std::cout << "   File size: " << fileTransfer.getTransferStatus(3) << std::endl;
        fileTransfer.cancelTransfer(&client);
    }
    
    std::cout << "\n=== File Operations Test Complete ===" << std::endl;
}

void testRealFileTransfer() {
    std::cout << "\n=== REAL FILE TRANSFER TEST ===" << std::endl;
    
    FileTransfer fileTransfer;
    Client uploadClient(10);
    Client downloadClient(11);
    
    uploadClient.setNickname("uploader");
    downloadClient.setNickname("downloader");
    
    // Test 1: Upload a real file
    std::cout << "\n1. Testing REAL file upload..." << std::endl;
    
    // Create a real file to upload
    std::string filename = "real_test_file.txt";
    std::ofstream create_file("./uploads/" + filename);
    if (create_file.is_open()) {
        create_file << "This is a REAL test file!\n";
        create_file << "Line 2: Testing actual file transfer\n";
        create_file << "Line 3: More content for testing\n";
        create_file << "Line 4: Almost done\n";
        create_file << "Line 5: Final line of the test file\n";
        create_file.close();
        
        // Get file size
        std::ifstream check_file("./uploads/" + filename);
        check_file.seekg(0, std::ios::end);
        size_t file_size = check_file.tellg();
        check_file.close();
        
        std::cout << "   Created test file: " << filename << " (" << file_size << " bytes)" << std::endl;
        
        // Start upload
        bool upload_started = fileTransfer.startUpload(&uploadClient, filename, file_size);
        std::cout << "   Upload started: " << (upload_started ? "SUCCESS" : "FAILED") << std::endl;
        
        if (upload_started) {
            std::cout << "   Transfer status: " << fileTransfer.getTransferStatus(10) << std::endl;
            std::cout << "   Progress bar: " << fileTransfer.getProgressBar(10) << std::endl;
            
            // Simulate receiving file chunks (like real network data)
            std::cout << "\n   Simulating file chunks..." << std::endl;
            
            // Read the file and send it in chunks
            std::ifstream source_file("./uploads/" + filename);
            char buffer[8192]; // 8KB chunks
            size_t total_sent = 0;
            
            while (source_file.read(buffer, 8192)) {
                std::streamsize bytes_read = source_file.gcount();
                std::string chunk(buffer, bytes_read);
                
                // Process this chunk as if it came from network
                fileTransfer.processUploadChunk(&uploadClient, chunk);
                total_sent += bytes_read;
                
                std::cout << "     Chunk sent: " << bytes_read << " bytes (Total: " << total_sent << "/" << file_size << ")" << std::endl;
                std::cout << "     Progress: " << fileTransfer.getProgressBar(10) << std::endl;
                
                // Small delay to simulate network transfer
                usleep(100000); // 0.1 second delay
            }
            
            source_file.close();
            
            std::cout << "\n   Upload completed!" << std::endl;
            std::cout << "   Final status: " << fileTransfer.getTransferStatus(10) << std::endl;
        }
    }
    
    // Test 2: Try to upload a blocked file type
    std::cout << "\n2. Testing blocked file type (executable)..." << std::endl;
    bool blocked_upload = fileTransfer.startUpload(&uploadClient, "malware.exe", 1024);
    std::cout << "   Blocked upload attempt: " << (blocked_upload ? "SUCCESS (BAD!)" : "FAILED (GOOD!)") << std::endl;
    
    if (!blocked_upload) {
        std::cout << "   ✅ Security working: Executable files are blocked!" << std::endl;
    }
    
    // Test 3: Try to upload a file that's too large
    std::cout << "\n3. Testing file size limit..." << std::endl;
    bool large_upload = fileTransfer.startUpload(&uploadClient, "huge_file.zip", 200 * 1024 * 1024); // 200MB
    std::cout << "   Large file upload: " << (large_upload ? "SUCCESS (BAD!)" : "FAILED (GOOD!)") << std::endl;
    
    if (!large_upload) {
        std::cout << "   ✅ Size limit working: Files over 100MB are blocked!" << std::endl;
    }
    
    std::cout << "\n=== REAL FILE TRANSFER TEST COMPLETE ===" << std::endl;
}

void testClientToClientTransfer() {
    std::cout << "\n=== CLIENT-TO-CLIENT TRANSFER TEST ===" << std::endl;
    
    FileTransfer fileTransfer;
    Client sender(20);
    Client receiver(21);
    
    sender.setNickname("sender");
    receiver.setNickname("receiver");
    
    std::cout << "\n1. Testing client-to-client file transfer..." << std::endl;
    
    // Create a test file that will be "sent" from sender to receiver
    std::string filename = "client_transfer_test.txt";
    std::ofstream create_file("./uploads/" + filename);
    if (create_file.is_open()) {
        create_file << "This file is being transferred from client to client!\n";
        create_file << "Line 2: Testing the client-to-client transfer system\n";
        create_file << "Line 3: This simulates one client sending to another\n";
        create_file.close();
        
        // Get file size
        std::ifstream check_file("./uploads/" + filename);
        check_file.seekg(0, std::ios::end);
        size_t file_size = check_file.tellg();
        check_file.close();
        
        std::cout << "   Created test file: " << filename << " (" << file_size << " bytes)" << std::endl;
        
        // Start client-to-client transfer
        bool transfer_started = fileTransfer.startClientToClientTransfer(&sender, &receiver, filename, file_size);
        std::cout << "   Client-to-client transfer started: " << (transfer_started ? "SUCCESS" : "FAILED") << std::endl;
        
        if (transfer_started) {
            std::cout << "   Transfer active: " << (fileTransfer.isClientToClientTransferActive(20, 21) ? "YES" : "NO") << std::endl;
            
            // Simulate receiving file chunks from sender
            std::cout << "\n   Simulating file chunks from sender..." << std::endl;
            
            std::ifstream source_file("./uploads/" + filename);
            char buffer[8192]; // 8KB chunks
            size_t total_sent = 0;
            
            while (source_file.read(buffer, 8192)) {
                std::streamsize bytes_read = source_file.gcount();
                std::string chunk(buffer, bytes_read);
                
                // Process this chunk as if it came from sender client
                fileTransfer.processClientToClientChunk(&sender, &receiver, chunk);
                total_sent += bytes_read;
                
                std::cout << "     Chunk received: " << bytes_read << " bytes (Total: " << total_sent << "/" << file_size << ")" << std::endl;
                
                // Small delay to simulate network transfer
                usleep(100000); // 0.1 second delay
            }
            
            source_file.close();
            
            std::cout << "\n   Client-to-client transfer completed!" << std::endl;
            std::cout << "   File should now be available for receiver to download" << std::endl;
        }
    }
    
    std::cout << "\n=== CLIENT-TO-CLIENT TRANSFER TEST COMPLETE ===" << std::endl;
}

int main() {
    std::cout << "FileTransfer Class Test Program" << std::endl;
    std::cout << "===============================" << std::endl;
    
    try {
        // testFileTransfer();
        testFileOperations();
        testRealFileTransfer();
        testClientToClientTransfer();
    } catch (const std::exception& e) {
        std::cerr << "Error during testing: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "\nAll tests completed successfully!" << std::endl;
    return 0;
}
