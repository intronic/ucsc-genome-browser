require File.dirname(__FILE__) + '/../test_helper'
require 'pipeline_controller'
require 'account_controller'

# Re-raise errors caught by the controller.
class PipelineController; def rescue_action(e) raise e end; end

class PipelineControllerTest < Test::Unit::TestCase

  # Be sure to include AuthenticatedTestHelper in test/test_helper.rb instead
  # Then, you can remove it from this and the units test.
  include AuthenticatedTestHelper

  fixtures :users

  def login(username, password)
    controller = @controller
    @controller = AccountController.new
    post :login, :login => username, :password => password
    assert session[:user]
    assert_response :redirect
    @controller = controller
  end

  def setup
    @controller = PipelineController.new
    @request    = ActionController::TestRequest.new
    @response   = ActionController::TestResponse.new
    login('quentin', 'test')
  end

  # Replace this with your real tests.
  def test_truth
    assert true
  end


end
